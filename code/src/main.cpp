#include "shell.h"
#include <cstring>   // cho strerror
#include <cerrno>    // cho errno
#include "parser.h"
#include "executor.h"
#include "builtins.h"
#include "signals.h"
#include "env.h"

#include <iostream>
#include <cstdlib>

// Biến toàn cục
int g_last_exit_status = 0;
bool g_running = true;


// Xử lý lỗi
const char* shell_strerror(ShellError code) {
    switch (code) {
        case SHELL_OK:              return "Thành công";
        case ERR_CMD_NOT_FOUND:     return "Không tìm thấy lệnh";
        case ERR_PERMISSION_DENIED: return "Không có quyền truy cập";
        case ERR_FILE_NOT_FOUND:    return "Không tìm thấy file hoặc thư mục";
        case ERR_SYNTAX_ERROR:      return "Lỗi cú pháp";
        case ERR_FORK_FAILED:       return "Lỗi fork";
        case ERR_EXEC_FAILED:       return "Lỗi thực thi";
        case ERR_PIPE_FAILED:       return "Lỗi tạo pipe";
        case ERR_REDIRECT_FAILED:   return "Lỗi chuyển hướng";
        case ERR_INVALID_ARGS:      return "Tham số không hợp lệ";
        default:                    return "Lỗi không xác định";
    }
}

void shell_error(ShellError code, const std::string& context) {
    std::cerr << "myshell: " << context << ": " << shell_strerror(code) << std::endl;
}

void shell_perror(const std::string& prefix) {
    std::cerr << "myshell: " << prefix << ": " << strerror(errno) << std::endl;
}


// Khởi tạo và dọn dẹp Shell
void shell_init() {
    // Khởi tạo biến môi trường
    init_environment();
    
    // Khởi tạo registry lệnh nội trú
    init_builtins();
    
    // Thiết lập xử lý tín hiệu
    setup_shell_signals();
}

void shell_cleanup() {
    // Dọn dẹp tài nguyên nếu cần
}

// Đọc dòng lệnh từ người dùng
std::string read_line() {
    std::string line;
    
    // In dấu nhắc lệnh
    std::cout << "myshell> " << std::flush;
    
    // Đọc dòng lệnh
    if (!std::getline(std::cin, line)) {
        // EOF (Ctrl+D)
        g_running = false;
        std::cout << std::endl;
        return "";
    }
    
    return line;
}

// Thực thi một dòng lệnh
void execute_line(const std::string& line) {
    // Bỏ qua dòng trống và comment
    if (line.empty() || line[0] == '#') {
        return;
    }
    
    // Phân tích dòng lệnh thành pipeline
    Pipeline pipeline = parse(line);
    
    if (pipeline.empty()) {
        return;
    }
    
    // Thực thi pipeline
    g_last_exit_status = execute_pipeline(pipeline);
}

// Vòng lặp chính của Shell
void shell_loop() {
    while (g_running) {
        std::string line = read_line();
        
        if (!g_running) {
            break;
        }
        
        execute_line(line);
    }
}

// Điểm vào chương trình
int main(int argc, char* argv[]) {
    // Khởi tạo shell
    shell_init();
    
    // Kiểm tra tùy chọn -c (thực thi lệnh và thoát)
    if (argc >= 3 && std::string(argv[1]) == "-c") {
        std::string command;
        for (int i = 2; i < argc; i++) {
            if (i > 2) command += " ";
            command += argv[i];
        }
        execute_line(command);
        shell_cleanup();
        return g_last_exit_status;
    }
    
    // Chế độ tương tác
    std::cout << "MyShell v1.0 - Gõ 'help' để xem danh sách lệnh" << std::endl;
    
    shell_loop();
    
    shell_cleanup();
    return g_last_exit_status;
}
