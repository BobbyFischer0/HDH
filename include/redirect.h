#ifndef REDIRECT_H
#define REDIRECT_H

#include "shell.h"

// ============================================================================
// I/O Redirection
// ============================================================================

// Setup input redirection (< file)
int redirect_input(const std::string& filename);

// Setup output redirection (> file or >> file)
int redirect_output(const std::string& filename, bool append);

// Setup error redirection (2> file)
int redirect_error(const std::string& filename);

// Restore standard file descriptors
void restore_standard_fds();

// Save standard file descriptors (for restoration)
void save_standard_fds();

#endif // REDIRECT_H
