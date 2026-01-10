# BÁO CÁO BÀI TẬP LỚN: XÂY DỰNG ỨNG DỤNG SHELL

**Đề tài 4:** Xây dựng ứng dụng shell (tương tự bash)

**Ngôn ngữ:** C/C++

---

## 1. CÁC CHỨC NĂNG

### 1.1 Thực thi lệnh

| Chức năng | Mô tả | Ví dụ |
|-----------|-------|-------|
| Lệnh ngoại trú | Thực thi chương trình bên ngoài bằng `fork()` + `execvp()` | `ls -la`, `cat file.txt` |
| Lệnh nội trú | Thực thi trong tiến trình shell | `cd`, `pwd`, `echo`, `exit` |

### 1.2 Redirection (Chuyển hướng I/O)

| Cú pháp | Mô tả |
|---------|-------|
| `cmd < file` | Đọc input từ file |
| `cmd > file` | Ghi output ra file (ghi đè) |
| `cmd >> file` | Ghi output ra file (nối tiếp) |
| `cmd 2> file` | Chuyển hướng stderr ra file |

### 1.3 Pipe (Đường ống)

| Cú pháp | Mô tả |
|---------|-------|
| `cmd1 \| cmd2` | Nối output của cmd1 vào input của cmd2 |
| `cmd1 \| cmd2 \| cmd3` | Hỗ trợ nhiều pipe liên tiếp |

### 1.4 Chạy lệnh nền

| Cú pháp | Mô tả |
|---------|-------|
| `cmd &` | Chạy lệnh trong background, shell tiếp tục nhận lệnh mới |

### 1.5 Lệnh nội trú (Built-in Commands)

| Lệnh | Mô tả |
|------|-------|
| `cd [dir]` | Thay đổi thư mục làm việc |
| `pwd` | In thư mục hiện tại |
| `echo [args]` | In văn bản ra màn hình |
| `export VAR=val` | Thiết lập biến môi trường |
| `unset VAR` | Xóa biến môi trường |
| `env` | Liệt kê tất cả biến môi trường |
| `exit [code]` | Thoát shell |
| `help` | Hiển thị trợ giúp |

### 1.6 Xử lý Quotes (Ngoặc)

| Loại | Mô tả | Ví dụ |
|------|-------|-------|
| Single quote `'...'` | Giữ nguyên nội dung (literal) | `echo '$HOME'` → `$HOME` |
| Double quote `"..."` | Cho phép mở rộng biến | `echo "$HOME"` → `/home/user` |
| Backslash `\` | Escape ký tự đặc biệt | `echo Hello\ World` |

### 1.7 Wildcards (Ký tự đại diện)

| Pattern | Mô tả |
|---------|-------|
| `*` | Khớp 0 hoặc nhiều ký tự bất kỳ |
| `?` | Khớp đúng 1 ký tự bất kỳ |

---

## 2. MỨC ĐỘ HOÀN THIỆN

| STT | Yêu cầu | Trạng thái |
|-----|---------|------------|
| 1 | Cho phép gõ lệnh nội trú hoặc ngoại trú để thực thi | ✅ Hoàn thành |
| 2 | Cho phép redirection và pipe | ✅ Hoàn thành |
| 3 | Cho phép chạy lệnh nền bằng `&` | ✅ Hoàn thành |
| 4 | Cài đặt một số lệnh nội trú (cd, pwd, echo,...) | ✅ Hoàn thành |
| 5 | Cho phép các loại ngoặc `'`, `"`, `\` | ✅ Hoàn thành |
| 6 | Wildcards (`?*`) | ✅ Hoàn thành |

**Tổng kết:** 6/6 yêu cầu đã hoàn thành (100%)

---

## 3. PHÂN CÔNG NHIỆM VỤ

| Phần | Thành viên | MSSV | Nhiệm vụ |
|------|------------|------------|----------|
| Part 1 | Phạm Đăng Bách | 20223871 | Vòng lặp chính của Shell, xử lý lỗi |
| Part 2 | Đoàn Văn Thưởng | 20223817 | Phân tích input, xử lý quotes, wildcards |
| Part 3 | Nguyễn Trương Hồng Đức | 20223919 | Thực thi lệnh, pipe, redirection |
| Part 4 | Nguyễn Cao Quang Anh | 20223849 | Các lệnh nội trú |
| Part 5 | Nguyễn Thế Hào | 20223959 | Xử lý tín hiệu, biến môi trường |

---

## 4. HƯỚNG DẪN CÀI ĐẶT, DỊCH VÀ SỬ DỤNG

### 4.1 Yêu cầu hệ thống

- **Hệ điều hành:** Linux/Unix hoặc WSL (Windows Subsystem for Linux)
- **Trình biên dịch:** g++ hỗ trợ C++17
- **Công cụ build:** make
- **Cài đặt (nếu chưa có):**
  ```bash
  sudo apt update
  sudo apt install g++ make
  ```

### 4.2 Hướng dẫn biên dịch

```bash
# Clone hoặc giải nén project vào thư mục

# Di chuyển vào thư mục project
cd HDH

# Biên dịch
make

# Xóa file build (nếu cần build lại)
make clean
make
```

### 4.3 Hướng dẫn sử dụng

```bash
# Chạy shell
./myshell

# Hoặc chạy 1 lệnh rồi thoát
./myshell -c "echo hello world"
```

### 4.4 Ví dụ sử dụng

```bash
# Khởi động shell
./myshell
MyShell v1.0 - Type 'help' for available commands

# Lệnh cơ bản
myshell> pwd
/home/user

myshell> cd /tmp
myshell> ls -la

# Pipe
myshell> ls | grep cpp | wc -l

# Redirection
myshell> echo "Hello" > output.txt
myshell> cat < output.txt

# Background
myshell> sleep 10 &
[12345] Running in background

# Wildcards
myshell> ls *.cpp

# Quotes
myshell> echo '$HOME'
$HOME
myshell> echo "$HOME"
/home/user

# Thoát
myshell> exit
```

---

## 5. CẤU TRÚC THƯ MỤC

```
HDH/
├── Makefile              # File cấu hình biên dịch
├── README.md             # File báo cáo này
├── include/              # Các file header (.h)
│   ├── shell.h           # Cấu trúc dữ liệu chính
│   ├── parser.h          # Khai báo parser
│   ├── executor.h        # Khai báo executor
│   ├── builtins.h        # Khai báo lệnh nội trú
│   ├── signals.h         # Khai báo xử lý tín hiệu
│   ├── env.h             # Khai báo biến môi trường
│   └── wildcard.h        # Khai báo wildcard
└── src/                  # Các file source code (.cpp)
    ├── main.cpp          # Entry point
    ├── parser.cpp        # Phân tích input
    ├── executor.cpp      # Thực thi lệnh
    ├── builtins.cpp      # Lệnh nội trú
    ├── signals.cpp       # Xử lý tín hiệu
    ├── env.cpp           # Quản lý biến môi trường
    └── wildcard.cpp      # Mở rộng wildcard
```

| Lỗi | Cách khắc phục |
|-----|----------------|
| "Command not found" | Đảm bảo chạy trên Linux/WSL, không phải Windows CMD |
| Build errors | Cài đặt g++: `sudo apt install g++` |
| Permission denied | Chạy lệnh: `chmod +x myshell` |

---

**Môn học:** Hệ điều hành

**Đề tài:** Xây dựng ứng dụng Shell
