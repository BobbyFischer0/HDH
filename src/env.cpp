#include "env.h"
#include <cstring>
#include <cstdlib>
#include <unistd.h>

// ============================================================================
// Environment Variable Storage
// ============================================================================

static std::map<std::string, std::string> g_env_vars;

// ============================================================================
// Initialize Environment from System
// ============================================================================

void init_environment() {
    extern char **environ;
    
    for (char **env = environ; *env != nullptr; env++) {
        std::string entry(*env);
        size_t pos = entry.find('=');
        if (pos != std::string::npos) {
            std::string name = entry.substr(0, pos);
            std::string value = entry.substr(pos + 1);
            g_env_vars[name] = value;
        }
    }
}

// ============================================================================
// Get Environment Variable
// ============================================================================

std::string get_env(const std::string& name) {
    auto it = g_env_vars.find(name);
    if (it != g_env_vars.end()) {
        return it->second;
    }
    return "";
}

// ============================================================================
// Set Environment Variable
// ============================================================================

void set_env(const std::string& name, const std::string& value) {
    g_env_vars[name] = value;
    
    // Also update the actual environment for child processes
    setenv(name.c_str(), value.c_str(), 1);
}

// ============================================================================
// Unset Environment Variable
// ============================================================================

void unset_env(const std::string& name) {
    g_env_vars.erase(name);
    
    // Also remove from actual environment
    unsetenv(name.c_str());
}

// ============================================================================
// Get All Environment Variables
// ============================================================================

const std::map<std::string, std::string>& get_all_env() {
    return g_env_vars;
}

// ============================================================================
// Build envp Array for execve
// ============================================================================

char** build_envp() {
    char** envp = new char*[g_env_vars.size() + 1];
    size_t i = 0;
    
    for (const auto& pair : g_env_vars) {
        std::string entry = pair.first + "=" + pair.second;
        envp[i] = new char[entry.size() + 1];
        strcpy(envp[i], entry.c_str());
        i++;
    }
    envp[i] = nullptr;
    
    return envp;
}

void free_envp(char** envp) {
    if (envp == nullptr) return;
    
    for (size_t i = 0; envp[i] != nullptr; i++) {
        delete[] envp[i];
    }
    delete[] envp;
}
