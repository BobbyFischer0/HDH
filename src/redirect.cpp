#include "redirect.h"

#include <fcntl.h>
#include <unistd.h>

// ============================================================================
// Saved File Descriptors
// ============================================================================

static int saved_stdin = -1;
static int saved_stdout = -1;
static int saved_stderr = -1;

// ============================================================================
// Save Standard File Descriptors
// ============================================================================

void save_standard_fds() {
    saved_stdin = dup(STDIN_FILENO);
    saved_stdout = dup(STDOUT_FILENO);
    saved_stderr = dup(STDERR_FILENO);
}

// ============================================================================
// Restore Standard File Descriptors
// ============================================================================

void restore_standard_fds() {
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
        saved_stdin = -1;
    }
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
        saved_stdout = -1;
    }
    if (saved_stderr != -1) {
        dup2(saved_stderr, STDERR_FILENO);
        close(saved_stderr);
        saved_stderr = -1;
    }
}

// ============================================================================
// Input Redirection
// ============================================================================

int redirect_input(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        return ERR_REDIRECT_FAILED;
    }
    
    dup2(fd, STDIN_FILENO);
    close(fd);
    
    return SHELL_OK;
}

// ============================================================================
// Output Redirection
// ============================================================================

int redirect_output(const std::string& filename, bool append) {
    int flags = O_WRONLY | O_CREAT;
    flags |= append ? O_APPEND : O_TRUNC;
    
    int fd = open(filename.c_str(), flags, 0644);
    if (fd == -1) {
        return ERR_REDIRECT_FAILED;
    }
    
    dup2(fd, STDOUT_FILENO);
    close(fd);
    
    return SHELL_OK;
}

// ============================================================================
// Error Redirection
// ============================================================================

int redirect_error(const std::string& filename) {
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        return ERR_REDIRECT_FAILED;
    }
    
    dup2(fd, STDERR_FILENO);
    close(fd);
    
    return SHELL_OK;
}
