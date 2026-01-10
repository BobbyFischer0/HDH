#include "parser.h"
#include "env.h"
#include "wildcard.h"
#include <unistd.h>   // for getpid

#include <sstream>
#include <cctype>

// ============================================================================
// Tokenizer Implementation
// ============================================================================

class Tokenizer {
public:
    Tokenizer(const std::string& input) : input_(input), pos_(0) {}
    
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        
        while (pos_ < input_.size()) {
            skip_whitespace();
            
            if (pos_ >= input_.size()) {
                break;
            }
            
            char c = input_[pos_];
            
            // Check for operators
            if (c == '|') {
                tokens.push_back(Token(TOKEN_PIPE, "|"));
                pos_++;
            }
            else if (c == '<') {
                tokens.push_back(Token(TOKEN_REDIRECT_IN, "<"));
                pos_++;
            }
            else if (c == '>') {
                pos_++;
                if (pos_ < input_.size() && input_[pos_] == '>') {
                    tokens.push_back(Token(TOKEN_REDIRECT_APPEND, ">>"));
                    pos_++;
                } else {
                    tokens.push_back(Token(TOKEN_REDIRECT_OUT, ">"));
                }
            }
            else if (c == '2' && pos_ + 1 < input_.size() && input_[pos_ + 1] == '>') {
                tokens.push_back(Token(TOKEN_REDIRECT_ERR, "2>"));
                pos_ += 2;
            }
            else if (c == '&') {
                tokens.push_back(Token(TOKEN_BACKGROUND, "&"));
                pos_++;
            }
            else if (c == '#') {
                // Comment - ignore rest of line
                break;
            }
            else {
                // Parse a word (possibly quoted)
                std::string word = parse_word();
                if (!word.empty()) {
                    tokens.push_back(Token(TOKEN_WORD, word));
                }
            }
        }
        
        tokens.push_back(Token(TOKEN_END, ""));
        return tokens;
    }
    
private:
    std::string input_;
    size_t pos_;
    
    void skip_whitespace() {
        while (pos_ < input_.size() && std::isspace(input_[pos_])) {
            pos_++;
        }
    }
    
    std::string parse_word() {
        std::string result;
        
        while (pos_ < input_.size()) {
            char c = input_[pos_];
            
            // Stop at whitespace or operators
            if (std::isspace(c) || c == '|' || c == '<' || c == '>' || 
                c == '&' || c == '#') {
                break;
            }
            
            // Handle 2> specially
            if (c == '2' && pos_ + 1 < input_.size() && input_[pos_ + 1] == '>') {
                break;
            }
            
            // Handle single quotes - preserve literally
            if (c == '\'') {
                pos_++;
                while (pos_ < input_.size() && input_[pos_] != '\'') {
                    result += input_[pos_];
                    pos_++;
                }
                if (pos_ < input_.size()) pos_++; // Skip closing quote
            }
            // Handle double quotes - allow escapes
            else if (c == '"') {
                pos_++;
                while (pos_ < input_.size() && input_[pos_] != '"') {
                    if (input_[pos_] == '\\' && pos_ + 1 < input_.size()) {
                        char next = input_[pos_ + 1];
                        if (next == '"' || next == '\\' || next == '$' || next == '`') {
                            result += next;
                            pos_ += 2;
                            continue;
                        }
                    }
                    result += input_[pos_];
                    pos_++;
                }
                if (pos_ < input_.size()) pos_++; // Skip closing quote
            }
            // Handle escape character
            else if (c == '\\') {
                pos_++;
                if (pos_ < input_.size()) {
                    result += input_[pos_];
                    pos_++;
                }
            }
            // Regular character
            else {
                result += c;
                pos_++;
            }
        }
        
        return result;
    }
};

std::vector<Token> tokenize(const std::string& line) {
    Tokenizer tokenizer(line);
    return tokenizer.tokenize();
}

// ============================================================================
// Variable Expansion
// ============================================================================

std::string expand_variables(const std::string& input) {
    std::string result;
    size_t i = 0;
    
    while (i < input.size()) {
        if (input[i] == '$' && i + 1 < input.size()) {
            i++;
            
            // $? - last exit status
            if (input[i] == '?') {
                result += std::to_string(g_last_exit_status);
                i++;
            }
            // $$ - shell PID
            else if (input[i] == '$') {
                result += std::to_string(getpid());
                i++;
            }
            // $VAR or ${VAR}
            else if (std::isalpha(input[i]) || input[i] == '_' || input[i] == '{') {
                bool braced = (input[i] == '{');
                if (braced) i++;
                
                std::string var_name;
                while (i < input.size() && (std::isalnum(input[i]) || input[i] == '_')) {
                    var_name += input[i];
                    i++;
                }
                
                if (braced && i < input.size() && input[i] == '}') {
                    i++;
                }
                
                result += get_env(var_name);
            }
            else {
                result += '$';
            }
        }
        else {
            result += input[i];
            i++;
        }
    }
    
    return result;
}

// ============================================================================
// Parser Implementation
// ============================================================================

Pipeline parse(const std::string& line) {
    Pipeline pipeline;
    std::vector<Token> tokens = tokenize(line);
    
    Command current_cmd;
    size_t i = 0;
    
    while (i < tokens.size() && tokens[i].type != TOKEN_END) {
        Token& tok = tokens[i];
        
        switch (tok.type) {
            case TOKEN_WORD: {
                // Expand variables
                std::string expanded = expand_variables(tok.value);
                
                // Expand wildcards
                if (has_wildcards(expanded)) {
                    std::vector<std::string> matches = expand_glob(expanded);
                    for (const auto& match : matches) {
                        current_cmd.args.push_back(match);
                    }
                } else {
                    current_cmd.args.push_back(expanded);
                }
                break;
            }
            
            case TOKEN_REDIRECT_IN:
                i++;
                if (i < tokens.size() && tokens[i].type == TOKEN_WORD) {
                    current_cmd.input_file = expand_variables(tokens[i].value);
                }
                break;
                
            case TOKEN_REDIRECT_OUT:
                i++;
                if (i < tokens.size() && tokens[i].type == TOKEN_WORD) {
                    current_cmd.output_file = expand_variables(tokens[i].value);
                    current_cmd.append_output = false;
                }
                break;
                
            case TOKEN_REDIRECT_APPEND:
                i++;
                if (i < tokens.size() && tokens[i].type == TOKEN_WORD) {
                    current_cmd.output_file = expand_variables(tokens[i].value);
                    current_cmd.append_output = true;
                }
                break;
                
            case TOKEN_REDIRECT_ERR:
                i++;
                if (i < tokens.size() && tokens[i].type == TOKEN_WORD) {
                    current_cmd.error_file = expand_variables(tokens[i].value);
                }
                break;
                
            case TOKEN_PIPE:
                if (!current_cmd.empty()) {
                    pipeline.commands.push_back(current_cmd);
                    current_cmd = Command();
                }
                break;
                
            case TOKEN_BACKGROUND:
                current_cmd.background = true;
                pipeline.background = true;
                break;
                
            default:
                break;
        }
        
        i++;
    }
    
    // Add last command
    if (!current_cmd.empty()) {
        pipeline.commands.push_back(current_cmd);
    }
    
    return pipeline;
}
