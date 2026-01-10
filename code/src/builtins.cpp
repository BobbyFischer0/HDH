#include "builtins.h"
#include "env.h"
#include "shell.h"

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <map>
#include <climits>


// Registry lệnh nội trú
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

// cd - Đổi thư mục
int builtin_cd(const std::vector<std::string>& args) {
    std::string target;
    
    if (args.size() < 2) {
        // Không có tham số - về thư mục HOME
        target = get_env("HOME");
        if (target.empty()) {
            std::cerr << "cd: HOME chưa được thiết lập" << std::endl;
            return 1;
        }
    } else if (args[1] == "-") {
        // cd - : quay về thư mục trước
        target = get_env("OLDPWD");
        if (target.empty()) {
            std::cerr << "cd: OLDPWD chưa được thiết lập" << std::endl;
            return 1;
        }
        std::cout << target << std::endl;
    } else if (args[1] == "~") {
        target = get_env("HOME");
        if (target.empty()) {
            std::cerr << "cd: HOME chưa được thiết lập" << std::endl;
            return 1;
        }
    } else {
        target = args[1];
    }
    
    // Lưu thư mục hiện tại vào OLDPWD
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        set_env("OLDPWD", cwd);
    }
    
    // Đổi thư mục
    if (chdir(target.c_str()) != 0) {
        shell_perror("cd: " + target);
        return 1;
    }
    
    // Cập nhật biến PWD
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        set_env("PWD", cwd);
    }
    
    return 0;
}


// pwd - In thư mục hiện tại
int builtin_pwd(const std::vector<std::string>& args) {
    (void)args;  // Không sử dụng
    
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << cwd << std::endl;
        return 0;
    } else {
        shell_perror("pwd");
        return 1;
    }
}


// echo - In văn bản ra màn hình
int builtin_echo(const std::vector<std::string>& args) {
    bool newline = true;
    size_t start = 1;
    
    // Kiểm tra cờ -n (không xuống dòng)
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

// exit - Thoát Shell
int builtin_exit(const std::vector<std::string>& args) {
    int exit_code = g_last_exit_status;
    
    if (args.size() > 1) {
        try {
            exit_code = std::stoi(args[1]);
        } catch (...) {
            std::cerr << "exit: " << args[1] << ": yêu cầu tham số là số" << std::endl;
            exit_code = 2;
        }
    }
    
    g_running = false;
    g_last_exit_status = exit_code;
    return exit_code;
}

// help - Hiển thị trợ giúp
int builtin_help(const std::vector<std::string>& args) {
    (void)args;
    
    std::cout << "MyShell v1.0 - Các lệnh nội trú:" << std::endl;
    std::cout << std::endl;
    std::cout << "  cd [dir]       Đổi thư mục (mặc định: HOME)" << std::endl;
    std::cout << "  pwd            In thư mục hiện tại" << std::endl;
    std::cout << "  echo [args]    In văn bản (-n để không xuống dòng)" << std::endl;
    std::cout << "  export VAR=val Thiết lập biến môi trường" << std::endl;
    std::cout << "  unset VAR      Xóa biến môi trường" << std::endl;
    std::cout << "  env            Liệt kê tất cả biến môi trường" << std::endl;
    std::cout << "  exit [code]    Thoát shell với mã thoát tùy chọn" << std::endl;
    std::cout << "  help           Hiển thị trợ giúp này" << std::endl;
    std::cout << std::endl;
    std::cout << "Các tính năng:" << std::endl;
    std::cout << "  cmd1 | cmd2    Pipe output của cmd1 sang cmd2" << std::endl;
    std::cout << "  cmd < file     Chuyển hướng input từ file" << std::endl;
    std::cout << "  cmd > file     Chuyển hướng output ra file" << std::endl;
    std::cout << "  cmd >> file    Nối output vào cuối file" << std::endl;
    std::cout << "  cmd 2> file    Chuyển hướng lỗi ra file" << std::endl;
    std::cout << "  cmd &          Chạy lệnh trong nền" << std::endl;
    std::cout << "  'text'         Ngoặc đơn (giữ nguyên)" << std::endl;
    std::cout << "  \"text\"         Ngoặc kép (mở rộng $vars)" << std::endl;
    std::cout << "  *.txt          Mở rộng wildcard" << std::endl;
    std::cout << std::endl;
    
    return 0;
}

// export - Thiết lập biến môi trường
int builtin_export(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        // Không có tham số - liệt kê tất cả biến
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
            // Chỉ export biến đã có (chưa triển khai)
        }
    }
    
    return 0;
}

// unset - Xóa biến môi trường
int builtin_unset(const std::vector<std::string>& args) {
    for (size_t i = 1; i < args.size(); i++) {
        unset_env(args[i]);
    }
    return 0;
}

// env - Liệt kê tất cả biến môi trường
int builtin_env(const std::vector<std::string>& args) {
    (void)args;
    
    for (const auto& pair : get_all_env()) {
        std::cout << pair.first << "=" << pair.second << std::endl;
    }
    
    return 0;
}
