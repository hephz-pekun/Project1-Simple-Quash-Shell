#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";

extern char **environ;

/* Ignore Ctrl-C in shell */
void sigint_handler(int sig)
{
    (void)sig;
    printf("\n");
    fflush(stdout);
}

int main()
{
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];

    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];

    signal(SIGINT, sigint_handler);

    while (true)
    {
        do
        {
            // Print current working directory prompt
            char cwd[1024];

            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("%s> ", cwd);
            else
                printf("%s", prompt);

            fflush(stdout);

            if ((fgets(command_line,
                       MAX_COMMAND_LINE_LEN,
                       stdin) == NULL) &&
                ferror(stdin))
            {
                fprintf(stderr, "fgets error");
                exit(0);
            }

        } while (command_line[0] == 0x0A);

        command_line[strlen(command_line) - 1] = '\0';

        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin))
        {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

        strcpy(cmd_bak, command_line);

        // 1. Tokenize the command line input (split it on whitespace)

        int argc = 0;

        char *token = strtok(command_line, delimiters);

        while (token != NULL &&
               argc < MAX_COMMAND_LINE_ARGS - 1)
        {
            /* Environment variable expansion */
            if (token[0] == '$')
            {
                char *value = getenv(token + 1);

                if (value)
                    arguments[argc++] = value;
                else
                    arguments[argc++] = "";
            }
            else
            {
                arguments[argc++] = token;
            }

            token = strtok(NULL, delimiters);
        }

        arguments[argc] = NULL;

        if (arguments[0] == NULL)
            continue;

        // 2. Implement Built-In Commands

        if (strcmp(arguments[0], "exit") == 0)
        {
            exit(0);
        }

        if (strcmp(arguments[0], "pwd") == 0)
        {
            char cwd[1024];

            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("%s\n", cwd);

            continue;
        }

        if (strcmp(arguments[0], "cd") == 0)
        {
            if (arguments[1] == NULL)
            {
                char *home = getenv("HOME");

                if (home != NULL)
                    chdir(home);
            }
            else
            {
                if (chdir(arguments[1]) != 0)
                    perror("cd");
            }

            continue;
        }

        if (strcmp(arguments[0], "echo") == 0)
        {
            for (int i = 1; arguments[i] != NULL; i++)
            {
                printf("%s", arguments[i]);

                if (arguments[i + 1] != NULL)
                    printf(" ");
            }

            printf("\n");
            continue;
        }

        if (strcmp(arguments[0], "env") == 0)
        {
            if (arguments[1] == NULL)
            {
                for (int i = 0; environ[i] != NULL; i++)
                    printf("%s\n", environ[i]);
            }
            else
            {
                char *value = getenv(arguments[1]);

                if (value)
                    printf("%s\n", value);
            }

            continue;
        }

        if (strcmp(arguments[0], "setenv") == 0)
        {
            if (arguments[1] && arguments[2])
            {
                setenv(arguments[1], arguments[2], 1);
            }
            else if (arguments[1] &&
                     strchr(arguments[1], '='))
            {
                char *eq = strchr(arguments[1], '=');

                *eq = '\0';

                setenv(arguments[1], eq + 1, 1);
            }

            continue;
        }

        // Background process support

        int background = 0;

        if (argc > 0 &&
            strcmp(arguments[argc - 1], "&") == 0)
        {
            background = 1;
            arguments[argc - 1] = NULL;
        }

        // Output redirection support

        char *outfile = NULL;
        char *infile = NULL;

        for (int i = 0; arguments[i] != NULL; i++)
        {
            if (strcmp(arguments[i], ">") == 0)
            {
                outfile = arguments[i + 1];
                arguments[i] = NULL;
                break;
            }

        for (int i = 0; arguments[i] != NULL; i++)
        {
            if (strcmp(arguments[i], "<") == 0)
            {
                infile = arguments[i + 1];
                arguments[i] = NULL;
                break;
            }

            if (arguments[i][0] == '<' &&
                strlen(arguments[i]) > 1)
            {
                infile = arguments[i] + 1;
                arguments[i] = NULL;
                break;
            }
        }

            /* handle >file */
            if (arguments[i][0] == '>' &&
                strlen(arguments[i]) > 1)
            {
                outfile = arguments[i] + 1;
                arguments[i] = NULL;
                break;
            }
        }

        // 3. Create a child process which will execute the command line input

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            continue;
        }

        if (pid == 0)
        {
            signal(SIGINT, SIG_DFL);

            if (infile != NULL)
            {
                int fd = open(infile, O_RDONLY);

                if (fd < 0)
                {
                    perror("open");
                    exit(1);
                }

                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            
            if (outfile != NULL)
            {
                int fd = open(outfile,
                              O_WRONLY | O_CREAT | O_TRUNC,
                              0644);

                if (fd < 0)
                {
                    perror("open");
                    exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(arguments[0], arguments);

            perror("execvp");
            exit(1);
        }

        // 4. Parent waits unless background process

        if (background)
        {
            printf("[background pid %d]\n", pid);
            continue;
        }

        int status;
        time_t start = time(NULL);

        while (1)
        {
            pid_t result =
                waitpid(pid,
                        &status,
                        WNOHANG);

            if (result == pid)
                break;

            if (result == -1)
                break;

            if (time(NULL) - start >= 10)
            {
                printf("Process exceeded 10 seconds. Killing process.\n");

                kill(pid, SIGKILL);

                waitpid(pid, &status, 0);

                break;
            }

            sleep(1);
        }

        // Hints (put these into Google):
        // man fork
        // man execvp
        // man wait
        // man strtok
        // man environ
        // man signals

        // Extra Credit
        // man dup2
        // man open
        // man pipes
    }

    // This should never be reached.
    return -1;
}