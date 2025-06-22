#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define MAX_JOBS 20

// Job status enumeration
typedef enum
{
    RUNNING,
    STOPPED,
    DONE
} JobStatus;

// Structure to hold job information
typedef struct
{
    pid_t pid;
    int job_id;
    char command[MAX_INPUT_SIZE];
    JobStatus status;
} Job;

// Global variables declaration
extern Job jobs[MAX_JOBS];
extern int job_count;

// Structure to hold command information
typedef struct
{
    char *args[MAX_ARGS];
    char input_file[MAX_INPUT_SIZE];
    char output_file[MAX_INPUT_SIZE];
    int append_output;
    int background;
    int pipe_count;
} Command;

// Function declarations
void initialize_shell(void);
void shell_loop(void);
char *read_line(void);
Command *parse_command(char *line);
int execute_command(Command *cmd);
int execute_builtin(Command *cmd);
void handle_signal(int signo);
void setup_signal_handlers(void);

// Built-in commands
int shell_cd(char **args);
int shell_pwd(void);
int shell_exit(void);
int shell_help(void);
int shell_jobs(void);
int shell_fg(char **args);
int shell_bg(char **args);
int shell_memstat(void);
int shell_memcheck(void);

// Job control functions
void add_job(pid_t pid, const char *command);
void remove_job(int job_id);
void update_job_status(void);
void print_jobs(void);

// Process management functions
pid_t create_process(Command *cmd);
void wait_for_process(pid_t pid);
void handle_background_process(pid_t pid, const char *command);

// I/O redirection functions
int setup_io_redirection(Command *cmd);
void reset_io_redirection(int stdin_copy, int stdout_copy);

// Environment variable functions
char *get_env_value(const char *name);
int set_env_value(const char *name, const char *value);

#endif /* SHELL_H */