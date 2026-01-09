#ifndef SIGNALS_H
#define SIGNALS_H
#include <csignal>
#include <sys/types.h>

// ============================================================================
// Signal Handling
// ============================================================================

// Current foreground process group (0 if none)
extern volatile sig_atomic_t g_foreground_pid;

// Setup signal handlers for the shell process
void setup_shell_signals();

// Setup default signal handlers for child processes
void setup_child_signals();

// SIGCHLD handler - reap zombie processes
void sigchld_handler(int sig);

// SIGINT handler - for the shell (ignore)
void sigint_handler(int sig);

#endif // SIGNALS_H
