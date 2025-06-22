## Project Overview

This project is a custom shell implementation in C, designed to demonstrate a wide range of operating system concepts. The shell supports command execution, job control, memory management, signal handling, I/O redirection, and more. The codebase is modular, with clear separation between memory management, process/job control, and shell logic.

## File-by-File Breakdown

### 1. `shell.c` and `shell.h` — The Shell Core

#### Features Implemented:
- **Command Parsing & Execution:** Reads user input, parses commands (including arguments, I/O redirection, background execution), and executes them.
- **Built-in Commands:** Implements `cd`, `pwd`, `exit`, `help`, `jobs`, `fg`, `bg`, `memstat`, and `memcheck`.
- **Job Control:** Tracks background and stopped jobs, assigns job IDs, and manages job status.
- **Signal Handling:** Handles `SIGINT` (Ctrl+C), `SIGTSTP` (Ctrl+Z), and `SIGCHLD` for process control and job status updates.
- **I/O Redirection:** Supports input (`<`), output (`>`), and append (`>>`) redirection.
- **Shell Loop:** Main loop for reading, parsing, and executing commands.

#### Concepts Used:
- **Process Management:** Uses `fork`, `exec`, and `wait` system calls to manage child processes.
- **Signal Handling:** Uses `signal()` to set up custom handlers for process control.
- **Job Control:** Maintains a job table, tracks process states (RUNNING, STOPPED, DONE), and provides job manipulation commands.
- **Memory Management:** Allocates memory for command structures and arguments, and integrates with a custom memory manager.
- **File System Operations:** Uses file descriptors and system calls for I/O redirection.
- **Environment Variables:** Functions for getting and setting environment variables (declared in header).

### 2. `process.c` — Process and Job Management

#### Features Implemented:
- **Process Creation:** Wraps `fork()` for process creation.
- **Process Waiting:** Waits for process termination or stop using `waitpid`.
- **Background Process Handling:** Adds background jobs to the job table.
- **I/O Redirection:** Sets up input/output redirection using `open`, `dup2`, and file descriptors.
- **Job Table Management:** Functions to add, remove, print, and update jobs.
- **Foreground/Background Control:** Implements `fg` and `bg` commands to move jobs between foreground and background.

#### Concepts Used:
- **System Calls:** `fork`, `waitpid`, `open`, `dup2`, `kill`, `signal`.
- **Job Control:** Maintains job status, job IDs, and command strings.
- **Process Synchronization:** Handles process state changes and updates job table accordingly.

### 3. `memory_manager.c` and `memory_manager.h` — Custom Memory Management

#### Features Implemented:
- **Memory Pool:** Initializes a fixed-size memory pool for dynamic allocations.
- **Custom Allocator:** Implements `shell_malloc`, `shell_free`, and `shell_realloc` for memory management within the pool.
- **Block Management:** Splits and merges memory blocks to minimize fragmentation.
- **Memory Statistics:** Tracks total allocated, freed, current usage, peak usage, and allocation/free counts.
- **Leak Detection:** Provides functions to check for memory leaks and print memory statistics.

#### Concepts Used:
- **Dynamic Memory Management:** Custom allocator mimics `malloc`/`free` using a memory pool and block list.
- **Fragmentation Handling:** Splits large blocks and merges adjacent free blocks.
- **Statistics & Debugging:** Tracks and reports memory usage and leaks.

### 4. `README.md` — Documentation & Testing

#### Features Documented:
- **Feature List:** Summarizes all shell features and OS concepts demonstrated.
- **Build & Run Instructions:** How to compile and run the shell.
- **Testing Guide:** Step-by-step test cases for all features (command execution, built-ins, I/O redirection, job control, etc.).
- **Expected Behaviors:** Describes correct shell behavior for each feature.
- **Troubleshooting:** Tips for resolving common issues.

---

## Key OS Concepts Demonstrated

1. **Process Control Block (PCB) Management:** Tracks process state, PID, command, and job status.
2. **System Calls:** Uses `fork`, `exec`, `wait`, `open`, `dup2`, `kill`, and others for process and file management.
3. **File Descriptors & I/O Handling:** Redirects input/output using low-level file operations.
4. **Signal Handling:** Custom handlers for process control and job management.
5. **Memory Management:** Custom allocator with statistics and leak detection.
6. **Environment Variable Management:** Functions for getting/setting environment variables (declarations present).
7. **Job Control:** Foreground/background execution, job table, and job manipulation commands.
8. **Shell Loop & Command Parsing:** Reads, parses, and executes user commands in a loop.

---

## Notable Implementation Details

- **Custom Memory Manager:** All dynamic allocations for commands and jobs can use the custom allocator, allowing for memory usage tracking and debugging.
- **Job Table:** Fixed-size array for job tracking, with job IDs and status for each process.
- **Signal Handling:** Ensures the shell remains responsive and robust to user interrupts and process state changes.
- **I/O Redirection:** Supports both input and output redirection, including append mode.
- **Testing & Documentation:** Comprehensive README with test cases and troubleshooting.

---

## Summary Table

| Feature                | File(s)              | Concepts Used                        |
|------------------------|----------------------|--------------------------------------|
| Command Execution      | shell.c, shell.h     | Parsing, fork/exec, memory mgmt      |
| Built-in Commands      | shell.c, shell.h     | String handling, process mgmt        |
| Job Control            | process.c, shell.c   | PCB, signals, job table              |
| I/O Redirection        | process.c, shell.c   | File descriptors, open/dup2          |
| Memory Management      | memory_manager.*     | Custom allocator, stats, leak check  |
| Signal Handling        | shell.c, process.c   | signal(), SIGINT, SIGTSTP, SIGCHLD   |
| Environment Variables  | shell.h (declared)   | getenv, setenv (not fully shown)     |
| File System Operations | process.c, shell.c   | open, close, chdir, getcwd           |

