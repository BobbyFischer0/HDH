#ifndef BUILTINS_H
#define BUILTINS_H

#include "shell.h"
#include <functional>
#include <map>
#include <string>

// ============================================================================
// Built-in Command Type
// ============================================================================
using BuiltinFunc = std::function<int(const std::vector<std::string>&)>;

// ============================================================================
// Built-in Commands
// ============================================================================
int builtin_cd(const std::vector<std::string>& args);
int builtin_pwd(const std::vector<std::string>& args);
int builtin_echo(const std::vector<std::string>& args);
int builtin_exit(const std::vector<std::string>& args);
int builtin_help(const std::vector<std::string>& args);
int builtin_export(const std::vector<std::string>& args);
int builtin_unset(const std::vector<std::string>& args);
int builtin_env(const std::vector<std::string>& args);

// ============================================================================
// Built-in Registry
// ============================================================================
bool is_builtin(const std::string& name);
BuiltinFunc get_builtin(const std::string& name);
void init_builtins();

#endif // BUILTINS_H
