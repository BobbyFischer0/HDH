# MyShell - A Bash-like Shell Implementation

A Unix-like shell implementation in C++ supporting command execution, pipes, redirection, background processes, and advanced input processing.

## Team Members & Responsibilities

This project is divided into 5 parts for team collaboration:

| Part | Member | Responsibility |
|------|--------|----------------|
| **Part 1** | Member 1 | Core Shell Loop & Error Handling |
| **Part 2** | Member 2 | Input Parsing, Quotes, Wildcards |
| **Part 3** | Member 3 | Command Execution, Pipes, Redirection |
| **Part 4** | Member 4 | Built-in Commands |
| **Part 5** | Member 5 | Signals & Environment Variables |

---

## Requirements

- **Operating System**: Linux/Unix or WSL (Windows Subsystem for Linux)
- **Compiler**: g++ with C++17 support
- **Build Tool**: make

---

## Quick Start

```bash
# Build the project
make

# Run the shell
./myshell

# Run with a single command
./myshell -c "echo hello world"

# Clean build files
make clean
```

---

## Part-by-Part Guide

### Part 1: Core Shell Loop & Error Handling (Member 1)


**What it does**:
- Main entry point and shell loop (read → parse → execute)
- Error codes and error handling functions
- Global state management

**How to test**:
```bash
./myshell
```

**Key functions**:
- `shell_init()` - Initialize the shell
- `shell_loop()` - Main read-eval-print loop
- `read_line()` - Read user input
- `shell_error()` - Display error messages

---

### Part 2: Input Parsing & Wildcards (Member 2)


**What it does**:
- Tokenize input into words and operators
- Handle single quotes `'...'` (literal)
- Handle double quotes `"..."` (allows `$vars`)
- Handle escape character `\`
- Expand wildcards `*` and `?`

**How to test**:
```bash
./myshell
echo hello world                    
echo 'single quotes $HOME'          
echo "double quotes $HOME"          
echo Hello\ World                   
echo *.cpp                          
echo ???.txt                        
```

**Key functions**:
- `tokenize()` - Split input into tokens
- `parse()` - Build Pipeline from tokens
- `expand_variables()` - Replace `$VAR` with values
- `expand_glob()` - Expand wildcard patterns

---

### Part 3: Command Execution & Pipes (Member 3)


**What it does**:
- Execute external commands with `fork()` and `execvp()`
- Create pipes for command chaining
- Handle I/O redirection (`<`, `>`, `>>`, `2>`)
- Support background execution (`&`)

**How to test**:
```bash
./myshell

# External commands
ls -la
cat /etc/passwd

# Pipes
ls | grep cpp
ls -la | grep src | wc -l
cat file.txt | sort | uniq

# Redirection
echo "hello" > output.txt         
cat < output.txt                  
echo "world" >> output.txt         
ls /nonexistent 2> error.txt       

# Background
sleep 5 &
echo "still running"
```

**Key functions**:
- `execute_pipeline()` - Execute commands connected by pipes
- `execute_command()` - Execute a single command
- `apply_redirections()` - Setup file redirections

**Pipe Algorithm**:
```
cmd1 | cmd2 | cmd3
  ↓      ↓      ↓
[write]→[read/write]→[read]
```

---

### Part 4: Built-in Commands (Member 4)


**What it does**:
- Commands that run in the shell process itself
- Cannot be external programs (like `cd`)

**How to test**:
```bash
./myshell

# Directory navigation
pwd                     
cd /tmp                 
cd -                   
cd ~                   

# Output
echo hello world        
echo -n "no newline"    

# Environment
export VAR=value        
echo $VAR               
env                     
unset VAR              

# Help & Exit
help                    
exit                    
exit 42               
```

**Built-in commands table**:
| Command | Description |
|---------|-------------|
| `cd [dir]` | Change directory |
| `pwd` | Print working directory |
| `echo [args]` | Print arguments |
| `export VAR=val` | Set environment variable |
| `unset VAR` | Remove environment variable |
| `env` | List all environment variables |
| `exit [code]` | Exit the shell |
| `help` | Show help |

---

### Part 5: Signals & Environment (Member 5)


**What it does**:
- Handle Ctrl+C (SIGINT) - stops child, not shell
- Handle Ctrl+Z (SIGTSTP) - suspend (ignored by shell)
- Reap zombie processes (SIGCHLD)
- Manage environment variables (`$VAR`, `$?`, `$$`)

**How to test**:
```bash
./myshell

# Signal handling
sleep 100              
# Press Ctrl+C          
echo "still here"       

# Environment variables
export NAME=MyShell
echo "Hello $NAME"      
echo "PID is $$"     

# Exit status
ls /nonexistent
echo "Exit code: $?"    
```

**Key functions**:
- `setup_shell_signals()` - Configure signal handlers for shell
- `setup_child_signals()` - Restore default handlers in children
- `sigchld_handler()` - Reap zombie processes
- `get_env()`, `set_env()`, `unset_env()` - Manage variables

---

## Project Structure

```
HDH/
├── Makefile                 # Build configuration
├── README.md               # This file
├── include/                # Header files
│   ├── shell.h            # Core structures
│   ├── parser.h           # Parser declarations
│   ├── executor.h         # Executor declarations
│   ├── builtins.h         # Built-in commands
│   ├── signals.h          # Signal handling
│   ├── env.h              # Environment variables
│   ├── redirect.h         # I/O redirection
│   └── wildcard.h         # Wildcard expansion
└── src/                    # Source files
    ├── main.cpp           # Entry point
    ├── parser.cpp         # Input parsing
    ├── executor.cpp       # Command execution
    ├── builtins.cpp       # Built-in commands
    ├── signals.cpp        # Signal handlers
    ├── env.cpp            # Environment management
    ├── redirect.cpp       # Redirection handling
    └── wildcard.cpp       # Glob patterns
```

---

## Building on Different Platforms

### Linux (Native)
```bash
make
./myshell
```

### Windows with WSL
```bash
wsl
cd /mnt/d/ComVisLab/HDH
make
./myshell
```

### macOS
```bash
make
./myshell
```

---

## Features Summary

| Feature | Syntax | Example |
|---------|--------|---------|
| Execute command | `cmd args` | `ls -la` |
| Pipe | `cmd1 \| cmd2` | `ls \| grep .cpp` |
| Input redirect | `cmd < file` | `sort < data.txt` |
| Output redirect | `cmd > file` | `echo hi > out.txt` |
| Append | `cmd >> file` | `echo hi >> out.txt` |
| Error redirect | `cmd 2> file` | `ls bad 2> err.txt` |
| Background | `cmd &` | `sleep 10 &` |
| Single quotes | `'text'` | `echo '$HOME'` |
| Double quotes | `"text"` | `echo "$HOME"` |
| Variables | `$VAR` | `echo $PATH` |
| Exit status | `$?` | `echo $?` |
| Shell PID | `$$` | `echo $$` |
| Wildcards | `*`, `?` | `ls *.cpp` |

---

## Troubleshooting

**"Command not found"**: Make sure to run in Linux/WSL, not Windows CMD.

**Build errors**: Ensure g++ is installed: `sudo apt install g++`

**Permission denied**: Run `chmod +x myshell` after building.

---

## License

Educational project for Operating Systems course.
