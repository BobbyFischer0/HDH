#ifndef ENV_H
#define ENV_H

#include <string>
#include <map>

// Quản lý biến môi trường
// Khởi tạo biến môi trường từ hệ thống
void init_environment();

// Lấy giá trị biến môi trường (rỗng nếu chưa thiết lập)
std::string get_env(const std::string& name);

// Thiết lập biến môi trường
void set_env(const std::string& name, const std::string& value);

// Xóa biến môi trường
void unset_env(const std::string& name);

// Lấy tất cả biến môi trường
const std::map<std::string, std::string>& get_all_env();

#endif // ENV_H
