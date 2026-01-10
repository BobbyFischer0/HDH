#include "env.h"
#include <cstring>
#include <cstdlib>
#include <unistd.h>


// Lưu trữ biến môi trường
static std::map<std::string, std::string> g_env_vars;


// Khởi tạo biến môi trường từ hệ thống
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

// Lấy giá trị biến môi trường
std::string get_env(const std::string& name) {
    auto it = g_env_vars.find(name);
    if (it != g_env_vars.end()) {
        return it->second;
    }
    return "";
}

// Thiết lập biến môi trường
void set_env(const std::string& name, const std::string& value) {
    g_env_vars[name] = value;
    
    // Cập nhật cả môi trường thực để tiến trình con kế thừa
    setenv(name.c_str(), value.c_str(), 1);
}


// Xóa biến môi trường
void unset_env(const std::string& name) {
    g_env_vars.erase(name);
    
    // Xóa khỏi môi trường thực
    unsetenv(name.c_str());
}

// Lấy tất cả biến môi trường
const std::map<std::string, std::string>& get_all_env() {
    return g_env_vars;
}
