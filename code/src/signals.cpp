#include "signals.h"
#include "shell.h"

#include <signal.h>
#include <sys/wait.h>
#include <iostream>
#include <cerrno>


// Biến toàn cục
volatile sig_atomic_t g_foreground_pid = 0;

// Xử lý SIGCHLD - Thu hồi tiến trình zombie
void sigchld_handler(int sig) {
    (void)sig;
    
    int saved_errno = errno;
    pid_t pid;
    int status;
    
    // Thu hồi tất cả tiến trình con đã kết thúc (không chặn)
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Có thể thông báo cho người dùng về job nền đã hoàn thành
        // Để đơn giản, chỉ thu hồi im lặng
    }
    
    errno = saved_errno;
}

// Xử lý SIGINT - Bỏ qua trong Shell
void sigint_handler(int sig) {
    (void)sig;
    // Shell bỏ qua SIGINT - chỉ tiến trình con nhận
    // In dòng mới để prompt gọn gàng hơn
    std::cout << std::endl;
}

// Thiết lập Signal Handler cho Shell
void setup_shell_signals() {
    struct sigaction sa;
    
    // SIGINT (Ctrl+C) - Handler tùy chỉnh
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, nullptr);
    
    // SIGTSTP (Ctrl+Z) - Bỏ qua trong shell
    signal(SIGTSTP, SIG_IGN);
    
    // SIGQUIT (Ctrl+\) - Bỏ qua trong shell
    signal(SIGQUIT, SIG_IGN);
    
    // SIGCHLD - Thu hồi tiến trình zombie
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, nullptr);
    
    // SIGTTOU - Bỏ qua (cho job control)
    signal(SIGTTOU, SIG_IGN);
    
    // SIGTTIN - Bỏ qua (cho job control)
    signal(SIGTTIN, SIG_IGN);
}

// Thiết lập Signal Handler mặc định cho tiến trình con
void setup_child_signals() {
    // Khôi phục signal handler mặc định cho tiến trình con
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
}
