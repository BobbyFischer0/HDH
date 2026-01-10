#ifndef WILDCARD_H
#define WILDCARD_H

#include <string>
#include <vector>


// Mở rộng Wildcard
// Kiểm tra pattern có chứa wildcard không
bool has_wildcards(const std::string& pattern);

// Mở rộng wildcard pattern thành danh sách file khớp
// Trả về pattern gốc nếu không có file khớp
std::vector<std::string> expand_glob(const std::string& pattern);

// So khớp chuỗi với pattern có * và ?
bool match_pattern(const std::string& pattern, const std::string& str);

#endif // WILDCARD_H
