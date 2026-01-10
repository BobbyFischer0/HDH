#include "builtins.h"
#include "env.h"
#include "shell.h"

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <map>
#include <climits>

// ============================================================================
// Built-in Registry
// ============================================================================

static std::map<std::string, BuiltinFunc> g_builtins;

void init_builtins() {
    g_builtins["cd"] = builtin_cd;
    g_builtins["pwd"] = builtin_pwd;
    g_builtins["echo"] = builtin_echo;
    g_builtins["exit"] = builtin_exit;
    g_builtins["help"] = builtin_help;
    g_builtins["export"] = builtin_export;
    g_builtins["unset"] = builtin_unset;
    g_builtins["env"] = builtin_env;
}

bool is_builtin(const std::string& name) {
    return g_builtins.find(name) != g_builtins.end();
}

BuiltinFunc get_builtin(const std::string& name) {
    auto it = g_builtins.find(name);
    if (it != g_builtins.end()) {
        return it->second;
    }
    return nullptr;
}

// ============================================================================
// cd - Change Directory
// ============================================================================

int builtin_cd(const std::vector<std::string>& args) {
    std::string target;
    
    if (args.size() < 2) {
        // No argument - go to HOME
        target = get_env("HOME");
        if (target.empty()) {
            std::cerr << "cd: HOME not set" << std::endl;
            return 1;
        }
    } else if (args[1] == "-") {
        // cd - : go to previous directory
        target = get_env("OLDPWD");
        if (target.empty()) {
            std::cerr << "cd: OLDPWD not set" << std::endl;
            return 1;
        }
        std::cout << target << std::endl;
    } else if (args[1] == "~") {
        target = get_env("HOME");
        if (target.empty()) {
            std::cerr << "cd: HOME not set" << std::endl;
            return 1;
        }
    } else {
        target = args[1];
    }
    
    // Save current directory as OLDPWD
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        set_env("OLDPWD", cwd);
    }
    
    // Change directory
    if (chdir(target.c_str()) != 0) {
        shell_perror("cd: " + target);
        return 1;
    }
    
    // Update PWD
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        set_env("PWD", cwd);
    }
    
    return 0;
}

// ============================================================================
// pwd - Print Working Directory
// ============================================================================

int builtin_pwd(const std::vector<std::string>& args) {
    (void)args;  // Unused
    
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << cwd << std::endl;
        return 0;
    } else {
        shell_perror("pwd");
        return 1;
    }
}

// ============================================================================
// echo - Print Arguments
// ============================================================================

int builtin_echo(const std::vector<std::string>& args) {
    bool newline = true;
    size_t start = 1;
    
    // Check for -n flag (no newline)
    if (args.size() > 1 && args[1] == "-n") {
        newline = false;
        start = 2;
    }
    
    for (size_t i = start; i < args.size(); i++) {
        if (i > start) {
            std::cout << " ";
        }
        std::cout << args[i];
    }
    
    if (newline) {
        std::cout << std::endl;
    }
    
    return 0;
}

// ============================================================================
// exit - Exit Shell
// ============================================================================

int builtin_exit(const std::vector<std::string>& args) {
    int exit_code = g_last_exit_status;
    
    if (args.size() > 1) {
        try {
            exit_code = std::stoi(args[1]);
        } catch (...) {
            std::cerr << "exit: " << args[1] << ": numeric argument required" << std::endl;
            exit_code = 2;
        }
    }
    
    g_running = false;
    g_last_exit_status = exit_code;
    return exit_code;
}

// ============================================================================
// help - Show Available Commands
// ============================================================================

int builtin_help(const std::vector<std::string>& args) {
    (void)args;
    
    std::cout << "MyShell v1.0 - Built-in Commands:" << std::endl;
    std::cout << std::endl;
    std::cout << "  cd [dir]       Change directory (default: HOME)" << std::endl;
    std::cout << "  pwd            Print working directory" << std::endl;
    std::cout << "  echo [args]    Print arguments (-n for no newline)" << std::endl;
    std::cout << "  export VAR=val Set environment variable" << std::endl;
    std::cout << "  unset VAR      Remove environment variable" << std::endl;
    std::cout << "  env            List environment variables" << std::endl;
    std::cout << "  exit [code]    Exit shell with optional exit code" << std::endl;
    std::cout << "  help           Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  cmd1 | cmd2    Pipe output of cmd1 to cmd2" << std::endl;
    std::cout << "  cmd < file     Redirect input from file" << std::endl;
    std::cout << "  cmd > file     Redirect output to file" << std::endl;
    std::cout << "  cmd >> file    Append output to file" << std::endl;
    std::cout << "  cmd 2> file    Redirect errors to file" << std::endl;
    std::cout << "  cmd &          Run command in background" << std::endl;
    std::cout << "  'text'         Single quotes (literal)" << std::endl;
    std::cout << "  \"text\"         Double quotes (allows $vars)" << std::endl;
    std::cout << "  *.txt          Wildcard expansion" << std::endl;
    std::cout << std::endl;
    
    return 0;
}

// ============================================================================
// export - Set Environment Variable
// ============================================================================

int builtin_export(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        // No arguments - show all exported variables
        for (const auto& pair : get_all_env()) {
            std::cout << "export " << pair.first << "=\"" << pair.second << "\"" << std::endl;
        }
        return 0;
    }
    
    for (size_t i = 1; i < args.size(); i++) {
        std::string arg = args[i];
        size_t eq_pos = arg.find('=');
        
        if (eq_pos != std::string::npos) {
            std::string name = arg.substr(0, eq_pos);
            std::string value = arg.substr(eq_pos + 1);
            set_env(name, value);
        } else {
            // Just export existing variable (no-op for now)
        }
    }
    
    return 0;
}

// ============================================================================
// unset - Remove Environment Variable
// ============================================================================

int builtin_unset(const std::vector<std::string>& args) {
    for (size_t i = 1; i < args.size(); i++) {
        unset_env(args[i]);
    }
    return 0;
}

// ============================================================================
// env - List Environment Variables
// ============================================================================

int builtin_env(const std::vector<std::string>& args) {
    (void)args;
    
    for (const auto& pair : get_all_env()) {
        std::cout << pair.first << "=" << pair.second << std::endl;
    }
    
    return 0;
}
