#ifndef ENV_H
#define ENV_H

#include <string>
#include <map>

// ============================================================================
// Environment Variable Management
// ============================================================================

// Initialize environment from system
void init_environment();

// Get environment variable value (empty if not set)
std::string get_env(const std::string& name);

// Set environment variable
void set_env(const std::string& name, const std::string& value);

// Unset environment variable
void unset_env(const std::string& name);

// Get all environment variables
const std::map<std::string, std::string>& get_all_env();

// Build envp array for execve (caller must free)
char** build_envp();
void free_envp(char** envp);

#endif // ENV_H
