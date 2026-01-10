#ifndef PARSER_H
#define PARSER_H

#include "shell.h"
#include <string>
#include <vector>

// ============================================================================
// Tokenizer
// ============================================================================

// Token types
enum TokenType {
    TOKEN_WORD,          // Regular word/argument
    TOKEN_PIPE,          // |
    TOKEN_REDIRECT_IN,   // <
    TOKEN_REDIRECT_OUT,  // >
    TOKEN_REDIRECT_APPEND, // >>
    TOKEN_REDIRECT_ERR,  // 2>
    TOKEN_BACKGROUND,    // &
    TOKEN_END            // End of input
};

struct Token {
    TokenType type;
    std::string value;
    
    Token(TokenType t = TOKEN_END, const std::string& v = "") 
        : type(t), value(v) {}
};

// ============================================================================
// Parser Functions
// ============================================================================

// Tokenize input line respecting quotes and escapes
std::vector<Token> tokenize(const std::string& line);

// Parse tokens into a pipeline of commands
Pipeline parse(const std::string& line);

// Expand environment variables in a string
std::string expand_variables(const std::string& input);

// Expand wildcards in arguments
std::vector<std::string> expand_wildcards(const std::string& pattern);

#endif // PARSER_H
