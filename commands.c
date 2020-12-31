#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "utils.c"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

const char *COMMANDS[] = {"ps_all", "search", "bookmark", "exit"};

/*
 * Checks the given *command parameter and returns the index
 * in the COMMANDS array.
 */
int check_command(char *command) {
    if (command == NULL)
        return 0;
    int len = sizeof(COMMANDS) / sizeof(*COMMANDS);
    int i;

    for (i = 0; i < len; i++) {
        if (strcmp(COMMANDS[i], command) == 0)
            return i + 1;
    }
    return 0;
}

void ps_all(struct process **background, struct process **finished, int *jobNumber) {
    printf("Running:\n");
    struct process *iterback = *background;
    while (iterback != NULL) {
        printf("\t[%d] %s (Pid=%d)\n", iterback->jobNumber, iterback->processName, (int) iterback->pid);
        iterback = iterback->next;
    }
    if (*background == NULL)
        *jobNumber = 1;
    printf("Finished:\n");
    struct process *iterfin = *finished;
    while (iterfin != NULL) {
        printf("\t[%d] %s\n", iterfin->jobNumber, iterfin->processName);
        iterfin = iterfin->next;
    }
    deleteList(finished);
}

/*
 * Before exiting, this functions
 * makes sure there is no background process
 * running.
 */
void check_exit(struct process **background_pids) {

    struct process *iter = *background_pids;
    if (iter == NULL)
        exit(0);
    else {
        printf("There are still running background applications.");
    }
}

/*
 * Determines if it is a file or directory.
 */
int is_regular_file(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

/*
 * isRecursive: boolean parameter
 * searchTerm: The given searchTerm
 * path: Recursively called path argument.
 */
void search(char *path, char *searchTerm, int isRecursive) {
    struct dirent *dp;
    DIR *dir = opendir(path);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL) {
        if (strstr(dp->d_name, ".c") != NULL || strstr(dp->d_name, ".h") != NULL ||
            strstr(dp->d_name, ".C") != NULL || strstr(dp->d_name, ".H") != NULL) {

            ssize_t line_size;
            char *line_buf = NULL;
            size_t line_buf_size = 0;

            char *copy = malloc(sizeof(char *));
            strcpy(copy, path);
            strcat(copy, "/");
            strcat(copy, dp->d_name);

            FILE *fp = fopen(copy, "r");
            line_size = getline(&line_buf, &line_buf_size, fp);
            int line_count = 0;

            while (line_size >= 0) {
                line_count++;

                if (strstr(line_buf, searchTerm)) {
                    printf("%d: %s -> %s\n", line_count, copy,
                           line_buf);
                }

                line_size = getline(&line_buf, &line_buf_size, fp);
            }

            free(line_buf);
            line_buf = NULL;

            fclose(fp);
        }


        if ((is_regular_file(dp->d_name) || dp->d_name[0] == '.')) {
            continue;
        } else {
            if (!isRecursive)
                continue;
            char *copy = malloc(sizeof(char *));
            strcpy(copy, path);
            strcat(copy, "/");
            strcat(copy, dp->d_name);
            search(copy, searchTerm, isRecursive);
        }
    }

    closedir(dir);
}

/*
 * This function gets called from main and determines
 * which function should be called and also supplies
 * the arguments background_pids and finished_pids
 */
void
run_command(int index, struct process **background_pids, struct process **finished_pids, int *jobNumber, char *args[]) {

    switch (index) {
        case 1:
            ps_all(background_pids, finished_pids, jobNumber);
            break;

        case 2:

            if (strcmp(args[1], "-r") == 0) {

                char *string = args[2];
                string++; // Remove first character the starting quote
                string[strlen(string) - 1] = '\0'; // Remove the last character the ending quote

                search(".", string, 1);
            } else {

                char *string = args[1];
                string++; // Remove first character the starting quote
                string[strlen(string) - 1] = '\0'; // Remove the last character the ending quote
                search(".", string, 0);
            }

            break;

        case 4:
            check_exit(background_pids);
            break;

        default:
            break;
    }
}

void handler(int signo) {
    if (foreground_pid == NULL)
        return;
    pid_t pid = foreground_pid->pid;
    deletePid(&foreground_pid, pid);
    kill(pid, SIGKILL);
}

void change_signal() {
    struct sigaction act;
    act.sa_handler = handler; /* set up signal handler */
    act.sa_flags = 0;
    if ((sigemptyset(&act.sa_mask) == -1) ||
        (sigaction(20, &act, NULL) == -1)) {
        perror("Failed to set SIGSTOP handler");
    }
}
