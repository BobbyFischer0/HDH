#include "wildcard.h"

#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <algorithm>

// Kiểm tra có wildcard không
bool has_wildcards(const std::string& pattern) {
    return pattern.find('*') != std::string::npos ||
           pattern.find('?') != std::string::npos;
}

// So khớp pattern (hỗ trợ * và ?)
bool match_pattern(const std::string& pattern, const std::string& str) {
    size_t p = 0, s = 0;
    size_t star_p = std::string::npos;
    size_t star_s = std::string::npos;
    
    while (s < str.size()) {
        if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == str[s])) {
            // Khớp một ký tự hoặc khớp chính xác
            p++;
            s++;
        }
        else if (p < pattern.size() && pattern[p] == '*') {
            // Tìm thấy dấu * - ghi nhớ vị trí
            star_p = p;
            star_s = s;
            p++;
        }
        else if (star_p != std::string::npos) {
            // Không khớp sau dấu * - quay lui
            p = star_p + 1;
            star_s++;
            s = star_s;
        }
        else {
            // Không khớp
            return false;
        }
    }
    
    // Kiểm tra phần pattern còn lại (chỉ nên là các dấu *)
    while (p < pattern.size() && pattern[p] == '*') {
        p++;
    }
    
    return p == pattern.size();
}

// Mở rộng Wildcard Pattern
std::vector<std::string> expand_glob(const std::string& pattern) {
    std::vector<std::string> results;
    
    // Tìm đường dẫn thư mục và pattern file
    size_t last_slash = pattern.rfind('/');
    std::string dir_path;
    std::string file_pattern;
    
    if (last_slash != std::string::npos) {
        dir_path = pattern.substr(0, last_slash + 1);
        file_pattern = pattern.substr(last_slash + 1);
    } else {
        dir_path = ".";
        file_pattern = pattern;
    }
    
    // Mở thư mục
    DIR* dir = opendir(dir_path.c_str());
    if (dir == nullptr) {
        // Trả về pattern gốc nếu không mở được thư mục
        results.push_back(pattern);
        return results;
    }
    
    // Đọc các entry trong thư mục
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        
        // Bỏ qua file ẩn trừ khi pattern bắt đầu bằng .
        if (name[0] == '.' && file_pattern[0] != '.') {
            continue;
        }
        
        // Bỏ qua . và ..
        if (name == "." || name == "..") {
            continue;
        }
        
        // Kiểm tra tên có khớp pattern không
        if (match_pattern(file_pattern, name)) {
            if (dir_path != ".") {
                results.push_back(dir_path + name);
            } else {
                results.push_back(name);
            }
        }
    }
    
    closedir(dir);
    
    // Sắp xếp kết quả
    std::sort(results.begin(), results.end());
    
    // Nếu không có kết quả, trả về pattern gốc
    if (results.empty()) {
        results.push_back(pattern);
    }
    
    return results;
}
