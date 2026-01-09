#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>

// ============================================================================
// Error Codes
// ============================================================================
enum ShellError {
    SHELL_OK = 0,
    ERR_CMD_NOT_FOUND = 127,
    ERR_PERMISSION_DENIED = 126,
    ERR_FILE_NOT_FOUND = 1,
    ERR_SYNTAX_ERROR = 2,
    ERR_FORK_FAILED = 3,
    ERR_EXEC_FAILED = 4,
    ERR_PIPE_FAILED = 5,
    ERR_REDIRECT_FAILED = 6,
    ERR_INVALID_ARGS = 7
};

// ============================================================================
// Data Structures
// ============================================================================
struct Command {
    std::vector<std::string> args;      // Command and arguments
    std::string input_file;              // Input redirection (<)
    std::string output_file;             // Output redirection (> or >>)
    bool append_output = false;          // true for >>, false for >
    std::string error_file;              // Error redirection (2>)
    bool background = false;             // Run in background (&)
    
    bool empty() const { return args.empty(); }
    std::string name() const { return args.empty() ? "" : args[0]; }
};

struct Pipeline {
    std::vector<Command> commands;       // Commands connected by pipes
    bool background = false;             // Entire pipeline in background
    
    bool empty() const { return commands.empty(); }
};

// ============================================================================
// Global State
// ============================================================================
extern int g_last_exit_status;           // Exit status of last command ($?)
extern bool g_running;                   // Main loop control

// ============================================================================
// Error Handling Functions
// ============================================================================
void shell_error(ShellError code, const std::string& context);
void shell_perror(const std::string& prefix);
const char* shell_strerror(ShellError code);

// ============================================================================
// Main Shell Functions
// ============================================================================
void shell_init();
void shell_cleanup();
void shell_loop();
std::string read_line();
void execute_line(const std::string& line);

#endif // SHELL_H
