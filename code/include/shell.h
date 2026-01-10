#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>


// Mã lỗi
enum ShellError {
    SHELL_OK = 0,
    ERR_CMD_NOT_FOUND = 127,
    ERR_PERMISSION_DENIED = 126,
    ERR_FILE_NOT_FOUND = 1,
    ERR_SYNTAX_ERROR = 2,
    ERR_FORK_FAILED = 3,
    ERR_EXEC_FAILED = 4,
    ERR_PIPE_FAILED = 5,
    ERR_REDIRECT_FAILED = 6,
    ERR_INVALID_ARGS = 7
};

// Cấu trúc dữ liệu
struct Command {
    std::vector<std::string> args;      // Lệnh và tham số
    std::string input_file;              // Chuyển hướng input (<)
    std::string output_file;             // Chuyển hướng output (> hoặc >>)
    bool append_output = false;          // true cho >>, false cho >
    std::string error_file;              // Chuyển hướng error (2>)
    bool background = false;             // Chạy trong nền (&)
    
    bool empty() const { return args.empty(); }
    std::string name() const { return args.empty() ? "" : args[0]; }
};

struct Pipeline {
    std::vector<Command> commands;       // Các lệnh nối bởi pipe
    bool background = false;             // Toàn bộ pipeline chạy nền
    
    bool empty() const { return commands.empty(); }
};


// Trạng thái toàn cục
extern int g_last_exit_status;           // Mã thoát của lệnh cuối ($?)
extern bool g_running;                   // Điều khiển vòng lặp chính


// Các hàm xử lý lỗi
void shell_error(ShellError code, const std::string& context);
void shell_perror(const std::string& prefix);
const char* shell_strerror(ShellError code);


// Các hàm Shell chính
void shell_init();
void shell_cleanup();
void shell_loop();
std::string read_line();
void execute_line(const std::string& line);

#endif // SHELL_H
