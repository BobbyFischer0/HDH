#ifndef PARSER_H
#define PARSER_H

#include "shell.h"
#include <string>
#include <vector>

// Tokenizer - Phân tích từ tố
// Các loại token
enum TokenType {
    TOKEN_WORD,          // Từ/tham số thường
    TOKEN_PIPE,          // |
    TOKEN_REDIRECT_IN,   // <
    TOKEN_REDIRECT_OUT,  // >
    TOKEN_REDIRECT_APPEND, // >>
    TOKEN_REDIRECT_ERR,  // 2>
    TOKEN_BACKGROUND,    // &
    TOKEN_END            // Kết thúc input
};

struct Token {
    TokenType type;
    std::string value;
    
    Token(TokenType t = TOKEN_END, const std::string& v = "") 
        : type(t), value(v) {}
};

// Các hàm Parser
// Phân tích dòng input thành token (xử lý quotes và escape)
std::vector<Token> tokenize(const std::string& line);

// Phân tích token thành pipeline các lệnh
Pipeline parse(const std::string& line);

// Mở rộng biến môi trường trong chuỗi
std::string expand_variables(const std::string& input);

// Mở rộng wildcard trong tham số
std::vector<std::string> expand_wildcards(const std::string& pattern);

#endif // PARSER_H
