#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "shell.h"

// ============================================================================
// Command Execution
// ============================================================================

// Execute a complete pipeline
int execute_pipeline(Pipeline& pipeline);

// Execute a single command (called internally)
int execute_command(Command& cmd, int input_fd, int output_fd);

// Execute a built-in command (returns true if command was a built-in)
bool execute_builtin(Command& cmd, int& exit_status);

// Apply I/O redirections for a command
int apply_redirections(const Command& cmd);

#endif // EXECUTOR_H
