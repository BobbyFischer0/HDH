#include "executor.h"
#include "builtins.h"
#include "signals.h"

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <vector>

// Áp dụng chuyển hướng I/O
int apply_redirections(const Command& cmd) {
    // Chuyển hướng input
    if (!cmd.input_file.empty()) {
        int fd = open(cmd.input_file.c_str(), O_RDONLY);
        if (fd == -1) {
            shell_perror(cmd.input_file);
            return ERR_REDIRECT_FAILED;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    // Chuyển hướng output
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
    
    // Chuyển hướng error
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


// Thực thi lệnh nội trú
bool execute_builtin(Command& cmd, int& exit_status) {
    if (cmd.empty()) {
        exit_status = 0;
        return true;
    }
    
    const std::string& name = cmd.name();
    
    if (!is_builtin(name)) {
        return false;
    }
    
    // Lấy hàm xử lý lệnh nội trú
    BuiltinFunc func = get_builtin(name);
    
    // Thực thi
    exit_status = func(cmd.args);
    
    return true;
}

// Thực thi một lệnh đơn
int execute_command(Command& cmd, int input_fd, int output_fd) {
    if (cmd.empty()) {
        return SHELL_OK;
    }
    
    // Kiểm tra lệnh nội trú (chỉ khi không có pipe)
    int builtin_status;
    if (input_fd == -1 && output_fd == -1 && execute_builtin(cmd, builtin_status)) {
        return builtin_status;
    }
    
    // Fork để thực thi lệnh ngoại trú
    pid_t pid = fork();
    
    if (pid == -1) {
        shell_perror("fork");
        return ERR_FORK_FAILED;
    }
    
    if (pid == 0) {
        // === TIẾN TRÌNH CON ===
        
        // Thiết lập signal handler mặc định
        setup_child_signals();
        
        // Thiết lập chuyển hướng pipe
        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        
        // Áp dụng chuyển hướng file (ghi đè pipe nếu có)
        if (apply_redirections(cmd) != SHELL_OK) {
            exit(ERR_REDIRECT_FAILED);
        }
        
        // Xây dựng mảng argument cho execvp
        std::vector<char*> argv;
        for (auto& arg : cmd.args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        
        // Thực thi lệnh
        execvp(argv[0], argv.data());
        
        // Nếu đến đây nghĩa là exec thất bại
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
    
    // === TIẾN TRÌNH CHA ===
    return pid;  // Trả về PID cho waitpid
}

// Thực thi Pipeline
int execute_pipeline(Pipeline& pipeline) {
    if (pipeline.empty()) {
        return SHELL_OK;
    }
    
    int n = pipeline.commands.size();
    
    // Lệnh đơn - có thể là builtin
    if (n == 1) {
        Command& cmd = pipeline.commands[0];
        
        // Thử builtin trước (các lệnh như cd phải chạy trong tiến trình cha)
        int builtin_status;
        if (execute_builtin(cmd, builtin_status)) {
            return builtin_status;
        }
        
        // Lệnh ngoại trú
        pid_t pid = execute_command(cmd, -1, -1);
        if (pid < 0) {
            return -pid;  // Mã lỗi
        }
        
        // Chờ hoàn thành (trừ khi chạy nền)
        if (!pipeline.background) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                return 128 + WTERMSIG(status);
            }
        } else {
            std::cout << "[" << pid << "] Đang chạy trong nền" << std::endl;
        }
        
        return SHELL_OK;
    }
    
    // Nhiều lệnh - tạo pipeline
    std::vector<pid_t> pids;
    int prev_pipe_read = -1;
    
    for (int i = 0; i < n; i++) {
        int pipefd[2] = {-1, -1};
        
        // Tạo pipe cho tất cả lệnh trừ lệnh cuối
        if (i < n - 1) {
            if (pipe(pipefd) == -1) {
                shell_perror("pipe");
                return ERR_PIPE_FAILED;
            }
        }
        
        // Fork tiến trình con
        pid_t pid = fork();
        
        if (pid == -1) {
            shell_perror("fork");
            // Đóng các pipe đang mở
            if (prev_pipe_read != -1) close(prev_pipe_read);
            if (pipefd[0] != -1) close(pipefd[0]);
            if (pipefd[1] != -1) close(pipefd[1]);
            return ERR_FORK_FAILED;
        }
        
        if (pid == 0) {
            // === TIẾN TRÌNH CON ===
            setup_child_signals();
            
            // Kết nối stdin với pipe trước (nếu không phải lệnh đầu tiên)
            if (prev_pipe_read != -1) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }
            
            // Kết nối stdout với pipe hiện tại (nếu không phải lệnh cuối)
            if (i < n - 1) {
                close(pipefd[0]);  // Đóng đầu đọc
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            
            // Áp dụng chuyển hướng file
            Command& cmd = pipeline.commands[i];
            if (apply_redirections(cmd) != SHELL_OK) {
                exit(ERR_REDIRECT_FAILED);
            }
            
            // Thực thi lệnh
            std::vector<char*> argv;
            for (auto& arg : cmd.args) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);
            
            execvp(argv[0], argv.data());
            
            // Exec thất bại
            if (errno == ENOENT) {
                shell_error(ERR_CMD_NOT_FOUND, cmd.name());
                exit(ERR_CMD_NOT_FOUND);
            } else {
                shell_perror(cmd.name());
                exit(ERR_EXEC_FAILED);
            }
        }
        
        // === TIẾN TRÌNH CHA ===
        pids.push_back(pid);
        
        // Đóng các đầu pipe đã dùng trong tiến trình cha
        if (prev_pipe_read != -1) {
            close(prev_pipe_read);
        }
        if (i < n - 1) {
            close(pipefd[1]);  // Đóng đầu ghi trong cha
            prev_pipe_read = pipefd[0];  // Lưu đầu đọc cho vòng lặp tiếp theo
        }
    }
    
    // Chờ tất cả tiến trình con (trừ khi chạy nền)
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
        std::cout << "[Pipeline] Đang chạy trong nền" << std::endl;
        return SHELL_OK;
    }
}
