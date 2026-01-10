#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "shell.h"

// Thực thi lệnh
// Thực thi toàn bộ pipeline
int execute_pipeline(Pipeline& pipeline);

// Thực thi một lệnh đơn (gọi nội bộ)
int execute_command(Command& cmd, int input_fd, int output_fd);

// Thực thi lệnh nội trú (trả về true nếu là lệnh nội trú)
bool execute_builtin(Command& cmd, int& exit_status);

// Áp dụng chuyển hướng I/O cho lệnh
int apply_redirections(const Command& cmd);

#endif // EXECUTOR_H
