#include "shell.h"
#include "memory_manager.h"

// Global variables - actual definition
Job jobs[MAX_JOBS];
int job_count = 0;
int shell_running = 1;
pid_t current_foreground_pid = 0;
char current_command[MAX_INPUT_SIZE] = ""; // Add this to track current command

void initialize_shell(void)
{
    // Initialize memory manager with 1MB pool
    init_memory_manager(1024 * 1024);

    // Set up signal handlers
    setup_signal_handlers();

    // Initialize jobs array
    for (int i = 0; i < MAX_JOBS; i++)
    {
        jobs[i].pid = 0;
        jobs[i].job_id = 0;
        jobs[i].status = DONE;
        jobs[i].command[0] = '\0';
    }
    job_count = 0;

    // Print welcome message
    printf("Welcome to MyShell!\n");
    printf("Type 'help' for a list of commands.\n");
}

void setup_signal_handlers(void)
{
    signal(SIGINT, handle_signal);
    signal(SIGTSTP, handle_signal);
    signal(SIGCHLD, handle_signal);
}

void handle_signal(int signo)
{
    switch (signo)
    {
    case SIGINT: // Ctrl+C
        if (current_foreground_pid > 0)
        {
            kill(current_foreground_pid, SIGINT);
            current_foreground_pid = 0;
            current_command[0] = '\0';
        }
        printf("\nchandan's shell> ");
        fflush(stdout);
        break;

    case SIGTSTP: // Ctrl+Z
        if (current_foreground_pid > 0)
        {
            // Terminate the foreground process
            kill(current_foreground_pid, SIGTERM);
            printf("\nTerminated: %s\n", current_command);
            current_foreground_pid = 0;
            current_command[0] = '\0';
        }
        printf("chandan's shell> ");
        fflush(stdout);
        break;

    case SIGCHLD:
        update_job_status();
        break;
    }
}

char *read_line(void)
{
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t characters;

    characters = getline(&line, &bufsize, stdin);

    if (characters == -1)
    {
        if (feof(stdin))
        {
            printf("\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror("getline");
            return NULL;
        }
    }

    // Remove trailing newline
    if (line[characters - 1] == '\n')
    {
        line[characters - 1] = '\0';
    }

    return line;
}

Command *parse_command(char *line)
{
    Command *cmd = malloc(sizeof(Command));
    if (!cmd)
    {
        perror("malloc");
        return NULL;
    }

    // Initialize command structure
    memset(cmd, 0, sizeof(Command));

    char *token;
    int i = 0;

    // Check for background execution
    if (line[strlen(line) - 1] == '&')
    {
        cmd->background = 1;
        line[strlen(line) - 1] = '\0';
    }

    // Parse input redirection
    char *input_redir = strchr(line, '<');
    if (input_redir)
    {
        *input_redir = '\0';
        sscanf(input_redir + 1, "%s", cmd->input_file);
    }

    // Parse output redirection
    char *output_redir = strchr(line, '>');
    if (output_redir)
    {
        if (*(output_redir + 1) == '>')
        {
            cmd->append_output = 1;
            *output_redir = '\0';
            sscanf(output_redir + 2, "%s", cmd->output_file);
        }
        else
        {
            *output_redir = '\0';
            sscanf(output_redir + 1, "%s", cmd->output_file);
        }
    }

    // Parse command and arguments
    token = strtok(line, " \t");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        cmd->args[i] = strdup(token);
        token = strtok(NULL, " \t");
        i++;
    }
    cmd->args[i] = NULL;

    return cmd;
}

int execute_command(Command *cmd)
{
    if (!cmd->args[0])
        return 1;

    // Check for built-in commands
    if (strcmp(cmd->args[0], "cd") == 0)
        return shell_cd(cmd->args);
    if (strcmp(cmd->args[0], "pwd") == 0)
        return shell_pwd();
    if (strcmp(cmd->args[0], "exit") == 0)
        return shell_exit();
    if (strcmp(cmd->args[0], "help") == 0)
        return shell_help();
    if (strcmp(cmd->args[0], "jobs") == 0)
        return shell_jobs();
    if (strcmp(cmd->args[0], "fg") == 0)
        return shell_fg(cmd->args);
    if (strcmp(cmd->args[0], "bg") == 0)
        return shell_bg(cmd->args);
    if (strcmp(cmd->args[0], "memstat") == 0)
        return shell_memstat();
    if (strcmp(cmd->args[0], "memcheck") == 0)
        return shell_memcheck();

    // Execute external command
    pid_t pid = create_process(cmd);

    if (pid == 0)
    {
        // Child process
        if (setup_io_redirection(cmd) != 0)
        {
            exit(EXIT_FAILURE);
        }

        // Reset signal handlers in child
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        execvp(cmd->args[0], cmd->args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    else
    {
        // Parent process
        // Build complete command string
        char cmd_str[MAX_INPUT_SIZE] = "";
        for (int i = 0; cmd->args[i] != NULL; i++)
        {
            if (i > 0)
                strcat(cmd_str, " ");
            strcat(cmd_str, cmd->args[i]);
        }

        if (cmd->background)
        {
            handle_background_process(pid, cmd_str);
        }
        else
        {
            current_foreground_pid = pid;
            strncpy(current_command, cmd_str, MAX_INPUT_SIZE - 1);
            current_command[MAX_INPUT_SIZE - 1] = '\0';
            wait_for_process(pid);
            current_foreground_pid = 0;
            current_command[0] = '\0';
        }
    }

    return 1;
}

int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        // Change to HOME directory
        char *home = getenv("HOME");
        if (home == NULL)
        {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
        if (chdir(home) != 0)
        {
            perror("cd");
            return 1;
        }
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("cd");
            return 1;
        }
    }
    return 1;
}

int shell_pwd(void)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
    }
    else
    {
        perror("pwd");
        return 1;
    }
    return 1;
}

int shell_exit(void)
{
    // Clean up any remaining jobs
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].status == RUNNING || jobs[i].status == STOPPED)
        {
            kill(jobs[i].pid, SIGTERM);
        }
    }

    // Check for memory leaks before exit
    check_memory_leaks();

    // Cleanup memory manager
    cleanup_memory_manager();

    printf("Goodbye!\n");
    shell_running = 0;
    return 0;
}

int shell_help(void)
{
    printf("MyShell - A simple shell implementation\n");
    printf("Built-in commands:\n");
    printf("  cd [dir]     Change directory\n");
    printf("  pwd          Print working directory\n");
    printf("  jobs         List background jobs\n");
    printf("  fg [job_id]  Bring job to foreground\n");
    printf("  bg [job_id]  Continue job in background\n");
    printf("  memstat      Display memory statistics\n");
    printf("  memcheck     Check for memory leaks\n");
    printf("  help         Display this help message\n");
    printf("  exit         Exit the shell\n");
    return 1;
}

// Add new built-in commands for memory management
int shell_memstat(void)
{
    print_memory_stats();
    print_memory_blocks();
    return 1;
}

int shell_memcheck(void)
{
    check_memory_leaks();
    return 1;
}

void shell_loop(void)
{
    char *line;
    Command *cmd;

    while (shell_running)
    {
        printf("chandan's shell> ");
        fflush(stdout);

        line = read_line();
        if (!line)
            continue;

        // Check for "exit" command directly
        if (strcmp(line, "exit") == 0)
        {
            free(line);
            shell_exit();
            break;
        }

        cmd = parse_command(line);
        if (!cmd)
        {
            free(line);
            continue;
        }

        execute_command(cmd);

        free(line);
        free(cmd);
    }
}

int main(void)
{
    initialize_shell();
    shell_loop();
    return EXIT_SUCCESS;
}