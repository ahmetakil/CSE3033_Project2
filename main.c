#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include "commands.c"

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define CREATE_FLAGS_APPEND (O_WRONLY | O_CREAT | O_APPEND)
#define CREATE_FLAGS_OVERWRITE (O_WRONLY | O_CREAT)
#define CREATE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[], int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0); /* ^d was entered, end of user command stream */

    /* the signal interrupted the read system call */
    /* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ((length < 0) && (errno != EINTR))
    {
        perror("error reading the command");
        exit(-1); /* terminate with error code of -1 */
    }

    for (i = 0; i < length; i++)
    { /* examine every character in the inputBuffer */

        switch (inputBuffer[i])
        {
        case ' ':
        case '\t': /* argument separators */
            if (start != -1)
            {
                args[ct] = &inputBuffer[start]; /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;

        case '\n': /* should be the final char examined */
            if (start != -1)
            {
                args[ct] = &inputBuffer[start];
                ct++;
            }
            inputBuffer[i] = '\0';
            args[ct] = NULL; /* no more arguments to this command */
            break;

        default: /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&')
            {
                *background = 1;
                inputBuffer[i - 1] = '\0';
            }
        }            /* end of switch */
    }                /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */

} /* end of setup routine */

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    pid_t child_pid;
    pid_t result;
    char inputBuffer[MAX_LINE];   /*buffer to hold command entered */
    int background;               /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE / 2 + 1]; /*command line arguments */
    int jobNumber = 1;
    struct process *background_pids = createProcess();
    struct process *finished_pids = createProcess();
    struct process *foreground_pids = createProcess();
    while (1)
    {
        background = 0;
        check_background(&background_pids, &finished_pids, &jobNumber);
        printf("myshell: ");
        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);
        build_command(args); // args without &

        char *command = args[0];
        char *processName = getProcessName(args);
        char *path = findPath(args[0]);

        int commandIndex = check_command(command);
        if (!commandIndex && path == NULL)
        {
            perror(("%s is not a valid command", command));
            continue;
        }
        if (!commandIndex)
            child_pid = fork();
        if (child_pid == -1)
        {
            perror("Failed to fork");
            return 1;
        }
        if (child_pid == 0 && path != NULL)
        {

            int index = 0;
            while (args[index] != NULL)
            {

                if (strcmp(args[index], ">") == 0)
                {

                    char *outputFile = args[index + 1];
                    int tableIndex = open(outputFile, CREATE_FLAGS_OVERWRITE, CREATE_MODE);
                    args[index] = NULL;

                    if (dup2(tableIndex, STDOUT_FILENO) == -1)
                    {
                        perror("Failed to close the file");
                        return 1;
                    }
                }
                else if (strcmp(args[index], ">>") == 0)
                {

                    char *outputFile = args[index + 1];
                    int tableIndex = open(outputFile, CREATE_FLAGS_APPEND, CREATE_MODE);
                    args[index] = NULL;

                    if (dup2(tableIndex, STDOUT_FILENO) == -1)
                    {
                        perror("Failed to close the file");
                        return 1;
                    }
                }
                else if (strcmp(args[index], "2>") == 0)
                {

                    char *outputFile = args[index + 1];
                    int tableIndex = open(outputFile, CREATE_FLAGS_APPEND, CREATE_MODE);
                    args[index] = NULL;

                    if (dup2(tableIndex, STDERR_FILENO) == -1)
                    {
                        perror("Failed to close the file");
                        return 1;
                    }
                }
                else if (strcmp(args[index], "<") == 0)
                {

                    char *outputFile = args[index + 1];
                    int tableIndex = open(outputFile, CREATE_FLAGS_APPEND, CREATE_MODE);
                    args[index] = NULL;

                    if (dup2(tableIndex, STDIN_FILENO) == -1)
                    {
                        perror("Failed to close the file");
                        return 1;
                    }
                }

                index++;
            }
            execv(path, &args[0]);
        }
        else
        {
            if (commandIndex)
            {
                run_command(commandIndex, &background_pids, &finished_pids, &jobNumber);
            }
        }

        switch (background)
        {
        case 0:
            child_pid = wait(NULL);
            if (child_pid == 0)
                insertPid(&foreground_pids, child_pid, processName, 0);
            break;
        case 1:
            insertPid(&background_pids, child_pid, processName, &jobNumber);
            break;
        }
    }
}