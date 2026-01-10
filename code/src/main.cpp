#include "shell.h"
#include <cstring>   // for strerror
#include <cerrno>    // for errno
#include "parser.h"
#include "executor.h"
#include "builtins.h"
#include "signals.h"
#include "env.h"

#include <iostream>
#include <cstdlib>

// ============================================================================
// Global Variables
// ============================================================================
int g_last_exit_status = 0;
bool g_running = true;

// ============================================================================
// Error Handling Implementation
// ============================================================================
const char* shell_strerror(ShellError code) {
    switch (code) {
        case SHELL_OK:              return "Success";
        case ERR_CMD_NOT_FOUND:     return "Command not found";
        case ERR_PERMISSION_DENIED: return "Permission denied";
        case ERR_FILE_NOT_FOUND:    return "No such file or directory";
        case ERR_SYNTAX_ERROR:      return "Syntax error";
        case ERR_FORK_FAILED:       return "Fork failed";
        case ERR_EXEC_FAILED:       return "Execution failed";
        case ERR_PIPE_FAILED:       return "Pipe creation failed";
        case ERR_REDIRECT_FAILED:   return "Redirection failed";
        case ERR_INVALID_ARGS:      return "Invalid arguments";
        default:                    return "Unknown error";
    }
}

void shell_error(ShellError code, const std::string& context) {
    std::cerr << "myshell: " << context << ": " << shell_strerror(code) << std::endl;
}

void shell_perror(const std::string& prefix) {
    std::cerr << "myshell: " << prefix << ": " << strerror(errno) << std::endl;
}

// ============================================================================
// Shell Initialization and Cleanup
// ============================================================================
void shell_init() {
    // Initialize environment variables
    init_environment();
    
    // Initialize built-in command registry
    init_builtins();
    
    // Setup signal handlers
    setup_shell_signals();
}

void shell_cleanup() {
    // Any cleanup needed
}

// ============================================================================
// Read Input Line
// ============================================================================
std::string read_line() {
    std::string line;
    
    // Print prompt
    std::cout << "myshell> " << std::flush;
    
    // Read line
    if (!std::getline(std::cin, line)) {
        // EOF (Ctrl+D)
        g_running = false;
        std::cout << std::endl;
        return "";
    }
    
    return line;
}

// ============================================================================
// Execute a Line
// ============================================================================
void execute_line(const std::string& line) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == '#') {
        return;
    }
    
    // Parse the line into a pipeline
    Pipeline pipeline = parse(line);
    
    if (pipeline.empty()) {
        return;
    }
    
    // Execute the pipeline
    g_last_exit_status = execute_pipeline(pipeline);
}

// ============================================================================
// Main Shell Loop
// ============================================================================
void shell_loop() {
    while (g_running) {
        std::string line = read_line();
        
        if (!g_running) {
            break;
        }
        
        execute_line(line);
    }
}

// ============================================================================
// Main Entry Point
// ============================================================================
int main(int argc, char* argv[]) {
    // Initialize shell
    shell_init();
    
    // Check for -c option (execute command and exit)
    if (argc >= 3 && std::string(argv[1]) == "-c") {
        std::string command;
        for (int i = 2; i < argc; i++) {
            if (i > 2) command += " ";
            command += argv[i];
        }
        execute_line(command);
        shell_cleanup();
        return g_last_exit_status;
    }
    
    // Interactive mode
    std::cout << "MyShell v1.0 - Type 'help' for available commands" << std::endl;
    
    shell_loop();
    
    shell_cleanup();
    return g_last_exit_status;
}
