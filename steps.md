# Custom Shell Implementation

A feature-rich shell implementation in C demonstrating various Operating System concepts.

## Features

- Basic command execution
- Built-in commands (cd, pwd, exit, help)
- Process Management
  - Foreground and background processes
  - Process creation and termination
  - Job control
- I/O Redirection
  - Input/Output redirection (>, <, >>)
  - Pipe implementation (|)
- Memory Management
  - Environment variables handling
  - Memory allocation for commands
- Signal Handling
  - Ctrl+C (SIGINT)
  - Ctrl+Z (SIGTSTP)
- File System Operations
  - File descriptors
  - Directory operations

## Building the Project

```bash
make
```

## Running the Shell

```bash
./myshell
```

## Implementation Details

This shell implements various OS concepts including:

1. Process Control Block (PCB) management
2. System calls (fork, exec, wait)
3. File descriptors and I/O handling
4. Signal handling and process synchronization
5. Memory management
6. Environment variable management

## Commands

- Built-in commands:
  - `cd [directory]`: Change directory
  - `pwd`: Print working directory
  - `exit`: Exit the shell
  - `help`: Display help information
  - `jobs`: List background jobs
  - `fg [job_id]`: Bring background job to foreground
  - `bg [job_id]`: Continue job in background

## Testing Guide

This section provides a comprehensive set of test cases to verify all shell functionalities.

### 1. Basic Command Execution

```bash
myshell> ls          # Should list files
myshell> date        # Should show current date
myshell> pwd         # Should show current directory
```

### 2. Built-in Commands

```bash
myshell> help        # Should show all available commands
myshell> pwd         # Should show current directory
myshell> cd /tmp     # Should change directory to /tmp
myshell> pwd         # Verify directory changed
myshell> cd ..       # Test relative path navigation
```

### 3. I/O Redirection

```bash
# Test output redirection
myshell> ls > files.txt
myshell> cat files.txt    # Should show directory contents

# Test input redirection
myshell> cat < files.txt  # Should display the file content

# Test append redirection
myshell> echo "hello" >> test.txt
myshell> echo "world" >> test.txt
myshell> cat test.txt     # Should show both lines
```

### 4. Background Processes

```bash
myshell> sleep 100 &      # Start a background process
myshell> jobs             # Should show the sleep process
myshell> sleep 200 &      # Start another background process
myshell> jobs             # Should show both processes
```

### 5. Job Control

```bash
# Start a process and stop it
myshell> sleep 100
# Press Ctrl+Z to stop it (Note: In this implementation, Ctrl+Z terminates the process)
myshell> jobs             # Should show process status
myshell> bg 1             # Continue in background
myshell> jobs             # Should show running process
myshell> fg 1             # Bring to foreground
# Press Ctrl+C to terminate
```

### Expected Behaviors

1. **Process Management**

   - Background processes should be properly tracked in the jobs list
   - Job IDs should be assigned sequentially
   - Process status should be accurately reported

2. **Signal Handling**

   - Ctrl+C (SIGINT): Terminates the current foreground process
   - Ctrl+Z (SIGTSTP): Terminates the current foreground process (modified behavior)
   - Shell should remain running after signal handling

3. **I/O Redirection**

   - File creation/appending should work as expected
   - Input redirection should read file contents correctly
   - Error messages should be displayed for invalid files

4. **Job Control**
   - `fg` command should properly resume background processes
   - `bg` command should continue stopped processes
   - `jobs` command should show accurate process status

### Troubleshooting

If you encounter any issues:

1. Check if the command exists and is in your PATH
2. Verify file permissions for I/O redirection
3. Ensure proper syntax for commands and arguments
4. Check job IDs when using fg/bg commands
