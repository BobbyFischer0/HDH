#include "signals.h"
#include "shell.h"

#include <signal.h>
#include <sys/wait.h>
#include <iostream>
#include <cerrno>

// ============================================================================
// Global Variables
// ============================================================================

volatile sig_atomic_t g_foreground_pid = 0;

// ============================================================================
// SIGCHLD Handler - Reap Zombie Processes
// ============================================================================

void sigchld_handler(int sig) {
    (void)sig;
    
    int saved_errno = errno;
    pid_t pid;
    int status;
    
    // Reap all terminated children (non-blocking)
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Could notify user about background job completion here
        // For simplicity, we just reap silently
    }
    
    errno = saved_errno;
}

// ============================================================================
// SIGINT Handler - Ignore in Shell
// ============================================================================

void sigint_handler(int sig) {
    (void)sig;
    // Shell ignores SIGINT - only children receive it
    // Print a newline for cleaner prompt
    std::cout << std::endl;
}

// ============================================================================
// Setup Signal Handlers for Shell Process
// ============================================================================

void setup_shell_signals() {
    struct sigaction sa;
    
    // SIGINT (Ctrl+C) - Custom handler
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, nullptr);
    
    // SIGTSTP (Ctrl+Z) - Ignore in shell
    signal(SIGTSTP, SIG_IGN);
    
    // SIGQUIT (Ctrl+\) - Ignore in shell
    signal(SIGQUIT, SIG_IGN);
    
    // SIGCHLD - Reap zombie processes
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, nullptr);
    
    // SIGTTOU - Ignore (for job control)
    signal(SIGTTOU, SIG_IGN);
    
    // SIGTTIN - Ignore (for job control)
    signal(SIGTTIN, SIG_IGN);
}

// ============================================================================
// Setup Default Signal Handlers for Child Processes
// ============================================================================

void setup_child_signals() {
    // Restore default signal handlers for child processes
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
}
