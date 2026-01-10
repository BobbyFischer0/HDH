#ifndef SIGNALS_H
#define SIGNALS_H
#include <csignal>
#include <sys/types.h>


// Xử lý tín hiệu
// PID của tiến trình foreground hiện tại (0 nếu không có)
extern volatile sig_atomic_t g_foreground_pid;

// Thiết lập signal handler cho tiến trình shell
void setup_shell_signals();

// Thiết lập signal handler mặc định cho tiến trình con
void setup_child_signals();

// Handler SIGCHLD - thu hồi tiến trình zombie
void sigchld_handler(int sig);

// Handler SIGINT - cho shell (bỏ qua)
void sigint_handler(int sig);

#endif // SIGNALS_H
