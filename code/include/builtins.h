#ifndef BUILTINS_H
#define BUILTINS_H

#include "shell.h"
#include <functional>
#include <map>
#include <string>

// Kiểu hàm cho lệnh nội trú
using BuiltinFunc = std::function<int(const std::vector<std::string>&)>;

// Các lệnh nội trú
int builtin_cd(const std::vector<std::string>& args);
int builtin_pwd(const std::vector<std::string>& args);
int builtin_echo(const std::vector<std::string>& args);
int builtin_exit(const std::vector<std::string>& args);
int builtin_help(const std::vector<std::string>& args);
int builtin_export(const std::vector<std::string>& args);
int builtin_unset(const std::vector<std::string>& args);
int builtin_env(const std::vector<std::string>& args);


// Registry lệnh nội trú
bool is_builtin(const std::string& name);
BuiltinFunc get_builtin(const std::string& name);
void init_builtins();

#endif // BUILTINS_H
