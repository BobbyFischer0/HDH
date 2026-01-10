#include "executor.h"
#include "builtins.h"
#include "signals.h"

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <vector>

// ============================================================================
// Apply Redirections
// ============================================================================

int apply_redirections(const Command& cmd) {
    // Input redirection
    if (!cmd.input_file.empty()) {
        int fd = open(cmd.input_file.c_str(), O_RDONLY);
        if (fd == -1) {
            shell_perror(cmd.input_file);
            return ERR_REDIRECT_FAILED;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    // Output redirection
    if (!cmd.output_file.empty()) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd.append_output ? O_APPEND : O_TRUNC;
        
        int fd = open(cmd.output_file.c_str(), flags, 0644);
        if (fd == -1) {
            shell_perror(cmd.output_file);
            return ERR_REDIRECT_FAILED;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    // Error redirection
    if (!cmd.error_file.empty()) {
        int fd = open(cmd.error_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            shell_perror(cmd.error_file);
            return ERR_REDIRECT_FAILED;
        }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
    
    return SHELL_OK;
}

// ============================================================================
// Execute Built-in Command
// ============================================================================

bool execute_builtin(Command& cmd, int& exit_status) {
    if (cmd.empty()) {
        exit_status = 0;
        return true;
    }
    
    const std::string& name = cmd.name();
    
    if (!is_builtin(name)) {
        return false;
    }
    
    // Get the built-in function
    BuiltinFunc func = get_builtin(name);
    
    // Execute it
    exit_status = func(cmd.args);
    
    return true;
}

// ============================================================================
// Execute Single Command
// ============================================================================

int execute_command(Command& cmd, int input_fd, int output_fd) {
    if (cmd.empty()) {
        return SHELL_OK;
    }
    
    // Check for built-in commands (only if no pipes)
    int builtin_status;
    if (input_fd == -1 && output_fd == -1 && execute_builtin(cmd, builtin_status)) {
        return builtin_status;
    }
    
    // Fork for external command
    pid_t pid = fork();
    
    if (pid == -1) {
        shell_perror("fork");
        return ERR_FORK_FAILED;
    }
    
    if (pid == 0) {
        // === CHILD PROCESS ===
        
        // Setup default signal handlers
        setup_child_signals();
        
        // Setup pipe redirections
        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        
        // Apply file redirections (overrides pipe if specified)
        if (apply_redirections(cmd) != SHELL_OK) {
            exit(ERR_REDIRECT_FAILED);
        }
        
        // Build argument array for execvp
        std::vector<char*> argv;
        for (auto& arg : cmd.args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        
        // Execute the command
        execvp(argv[0], argv.data());
        
        // If we get here, exec failed
        if (errno == ENOENT) {
            shell_error(ERR_CMD_NOT_FOUND, cmd.name());
            exit(ERR_CMD_NOT_FOUND);
        } else if (errno == EACCES) {
            shell_error(ERR_PERMISSION_DENIED, cmd.name());
            exit(ERR_PERMISSION_DENIED);
        } else {
            shell_perror(cmd.name());
            exit(ERR_EXEC_FAILED);
        }
    }
    
    // === PARENT PROCESS ===
    return pid;  // Return PID for waitpid
}

// ============================================================================
// Execute Pipeline
// ============================================================================

int execute_pipeline(Pipeline& pipeline) {
    if (pipeline.empty()) {
        return SHELL_OK;
    }
    
    int n = pipeline.commands.size();
    
    // Single command - might be a builtin
    if (n == 1) {
        Command& cmd = pipeline.commands[0];
        
        // Try builtin first (for commands like cd that must run in parent)
        int builtin_status;
        if (execute_builtin(cmd, builtin_status)) {
            return builtin_status;
        }
        
        // External command
        pid_t pid = execute_command(cmd, -1, -1);
        if (pid < 0) {
            return -pid;  // Error code
        }
        
        // Wait for completion (unless background)
        if (!pipeline.background) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                return 128 + WTERMSIG(status);
            }
        } else {
            std::cout << "[" << pid << "] Running in background" << std::endl;
        }
        
        return SHELL_OK;
    }
    
    // Multiple commands - create pipeline
    std::vector<pid_t> pids;
    int prev_pipe_read = -1;
    
    for (int i = 0; i < n; i++) {
        int pipefd[2] = {-1, -1};
        
        // Create pipe for all but last command
        if (i < n - 1) {
            if (pipe(pipefd) == -1) {
                shell_perror("pipe");
                return ERR_PIPE_FAILED;
            }
        }
        
        // Fork child
        pid_t pid = fork();
        
        if (pid == -1) {
            shell_perror("fork");
            // Close any open pipes
            if (prev_pipe_read != -1) close(prev_pipe_read);
            if (pipefd[0] != -1) close(pipefd[0]);
            if (pipefd[1] != -1) close(pipefd[1]);
            return ERR_FORK_FAILED;
        }
        
        if (pid == 0) {
            // === CHILD PROCESS ===
            setup_child_signals();
            
            // Connect stdin to previous pipe (if not first command)
            if (prev_pipe_read != -1) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }
            
            // Connect stdout to current pipe (if not last command)
            if (i < n - 1) {
                close(pipefd[0]);  // Close read end
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            
            // Apply file redirections
            Command& cmd = pipeline.commands[i];
            if (apply_redirections(cmd) != SHELL_OK) {
                exit(ERR_REDIRECT_FAILED);
            }
            
            // Execute command
            std::vector<char*> argv;
            for (auto& arg : cmd.args) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);
            
            execvp(argv[0], argv.data());
            
            // Exec failed
            if (errno == ENOENT) {
                shell_error(ERR_CMD_NOT_FOUND, cmd.name());
                exit(ERR_CMD_NOT_FOUND);
            } else {
                shell_perror(cmd.name());
                exit(ERR_EXEC_FAILED);
            }
        }
        
        // === PARENT PROCESS ===
        pids.push_back(pid);
        
        // Close used pipe ends in parent
        if (prev_pipe_read != -1) {
            close(prev_pipe_read);
        }
        if (i < n - 1) {
            close(pipefd[1]);  // Close write end in parent
            prev_pipe_read = pipefd[0];  // Save read end for next iteration
        }
    }
    
    // Wait for all children (unless background)
    if (!pipeline.background) {
        int last_status = 0;
        for (pid_t pid : pids) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                last_status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                last_status = 128 + WTERMSIG(status);
            }
        }
        return last_status;
    } else {
        std::cout << "[Pipeline] Running in background" << std::endl;
        return SHELL_OK;
    }
}
