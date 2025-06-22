#include "shell.h"

pid_t create_process(Command *cmd)
{
    // Explicitly mark parameter as unused
    (void)cmd;
    return fork();
}

void wait_for_process(pid_t pid)
{
    int status;
    // Wait for either process termination or stop
    while (waitpid(pid, &status, WUNTRACED) > 0)
    {
        if (WIFSTOPPED(status))
        {
            // Process was stopped, break the wait
            break;
        }
        else if (WIFEXITED(status) || WIFSIGNALED(status))
        {
            // Process terminated
            break;
        }
    }
}

void handle_background_process(pid_t pid, const char *command)
{
    add_job(pid, command);
}

int setup_io_redirection(Command *cmd)
{
    int fd;

    // Handle input redirection
    if (strlen(cmd->input_file) > 0)
    {
        fd = open(cmd->input_file, O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }

    // Handle output redirection
    if (strlen(cmd->output_file) > 0)
    {
        int flags = O_WRONLY | O_CREAT;
        if (cmd->append_output)
        {
            flags |= O_APPEND;
        }
        else
        {
            flags |= O_TRUNC;
        }

        fd = open(cmd->output_file, flags, 0644);
        if (fd == -1)
        {
            perror("open");
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }

    return 0;
}

void reset_io_redirection(int stdin_copy, int stdout_copy)
{
    if (dup2(stdin_copy, STDIN_FILENO) == -1)
    {
        perror("dup2");
    }
    if (dup2(stdout_copy, STDOUT_FILENO) == -1)
    {
        perror("dup2");
    }
    close(stdin_copy);
    close(stdout_copy);
}

int shell_jobs(void)
{
    print_jobs();
    return 1;
}

void add_job(pid_t pid, const char *command)
{
    int i;
    // First check if the process is already in jobs list
    for (i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].pid == pid)
        {
            // Update existing job
            jobs[i].status = STOPPED;
            return;
        }
    }

    // If not found, find an empty slot
    for (i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].status == DONE)
        {
            jobs[i].pid = pid;
            jobs[i].job_id = i + 1;
            strncpy(jobs[i].command, command, MAX_INPUT_SIZE - 1);
            jobs[i].command[MAX_INPUT_SIZE - 1] = '\0';
            jobs[i].status = RUNNING;
            job_count++;
            if (jobs[i].status == RUNNING)
            {
                printf("[%d] %d %s &\n", jobs[i].job_id, pid, command);
            }
            return;
        }
    }
    fprintf(stderr, "Maximum number of jobs reached\n");
}

void remove_job(int job_id)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].job_id == job_id)
        {
            jobs[i].pid = 0;
            jobs[i].job_id = 0;
            jobs[i].status = DONE;
            jobs[i].command[0] = '\0';
            job_count--;
            return;
        }
    }
}

void print_jobs(void)
{
    int found = 0;
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].status == RUNNING || jobs[i].status == STOPPED)
        {
            printf("[%d] %s %s\n",
                   jobs[i].job_id,
                   jobs[i].status == RUNNING ? "Running" : "Stopped",
                   jobs[i].command);
            found = 1;
        }
    }
    if (!found)
    {
        printf("No active jobs\n");
    }
}

int shell_fg(char **args)
{
    if (!args[1])
    {
        fprintf(stderr, "fg: job id required\n");
        return 1;
    }

    int job_id = atoi(args[1]);
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].job_id == job_id && jobs[i].status != DONE)
        {
            pid_t pid = jobs[i].pid;

            // Continue the process if it was stopped
            if (jobs[i].status == STOPPED)
            {
                kill(pid, SIGCONT);
                printf("%s\n", jobs[i].command);
            }

            // Wait for the process
            wait_for_process(pid);
            remove_job(job_id);
            return 1;
        }
    }

    fprintf(stderr, "fg: job %d not found\n", job_id);
    return 1;
}

int shell_bg(char **args)
{
    if (!args[1])
    {
        fprintf(stderr, "bg: job id required\n");
        return 1;
    }

    int job_id = atoi(args[1]);
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].job_id == job_id && jobs[i].status == STOPPED)
        {
            kill(jobs[i].pid, SIGCONT);
            jobs[i].status = RUNNING;
            printf("[%d] %s &\n", job_id, jobs[i].command);
            return 1;
        }
    }

    fprintf(stderr, "bg: job %d not found\n", job_id);
    return 1;
}

void update_job_status(void)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    {
        for (int i = 0; i < MAX_JOBS; i++)
        {
            if (jobs[i].pid == pid)
            {
                if (WIFSTOPPED(status))
                {
                    jobs[i].status = STOPPED;
                    printf("[%d] Stopped %s\n", jobs[i].job_id, jobs[i].command);
                }
                else if (WIFEXITED(status) || WIFSIGNALED(status))
                {
                    if (jobs[i].status != DONE)
                    {
                        printf("[%d] Done %s\n", jobs[i].job_id, jobs[i].command);
                        remove_job(jobs[i].job_id);
                    }
                }
                break;
            }
        }
    }
}