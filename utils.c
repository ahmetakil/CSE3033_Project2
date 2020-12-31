#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LINE 80

struct process *background_pids;
struct process *finished_pids;
struct process *foreground_pid;

struct process
{
    pid_t pid;
    char *processName;
    int jobNumber;
    struct process *next;
};
typedef struct process process;

char *concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char *findPath(char *command)
{
    if (command == NULL)
        return NULL;
    char *paths;
    paths = getenv("PATH");

    int length = strlen(paths);
    char *temp = malloc(length);
    memcpy(temp, paths, length);
    char *token;

    DIR *d;
    struct dirent *dir;
    bool finded = false;
    while ((token = strtok_r(temp, ":", &temp)))
    {

        if ((d = opendir(token)) != NULL)
            while ((dir = readdir(d)) != NULL)
            {
                if (strcmp(dir->d_name, command) == 0)
                {
                    finded = true;
                    break;
                }
            }
        if (finded)
            break;
    }
    char *path = NULL;
    if (token != NULL)
    {
        path = concat("/", command);
        path = concat(token, path);
    }

    return path;
}

void build_command(char *args[])
{
    int i = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i], "&") == 0)
        {
            int j = i;
            while (args[j] != NULL)
            {
                args[j] = "\0";
                args[j] = args[j + 1];
                j++;
            }
        }
        i++;
    }
}

process *createProcess()
{
    struct process *new = NULL;
    return new;
}

char *getProcessName(char *args[])
{
    char *processName = malloc(50);
    int i = 0;
    while (args[i] != NULL)
    {
        processName = concat(processName, args[i]);
        processName = concat(processName, " ");
        i++;
    }
    return processName;
}

void insertPid(struct process **processList, pid_t pid, char *processName, int *jobNumber, bool background)
{
    struct process *new = malloc(sizeof(struct process));
    new->pid = pid;
    new->processName = processName;
    if (background)
    {
        new->jobNumber = *jobNumber;
        *jobNumber += 1;
    }
    new->next = NULL;
    if (*processList == NULL)
        *processList = new;
    else
    {
        struct process *iter = *processList;
        while (iter->next != NULL)
            iter = iter->next;
        iter->next = new;
    }
}

void deletePid(struct process **processList, pid_t pid)
{
    struct process *temp = *processList;
    if (temp != NULL && temp->pid == pid)
    {
        *processList = temp->next;
        free(temp);
    }
    struct process *prev;
    while (temp != NULL && temp->pid != pid)
    {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL)
        return;
    prev->next = temp->next;
    free(temp);
}

void deleteList(struct process **processList)
{
    struct process *current = *processList;
    struct process *next;
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
    *processList = NULL;
}

void check_background_process(struct process **backgroundList, struct process **finishedList, int *jobNumber)
{
    pid_t pid;
    struct process *iter = *backgroundList;
    if (iter == NULL)
        return;
    char *processName;
    while (iter != NULL)
    {
        processName = iter->processName;
        pid = waitpid(iter->pid, NULL, WNOHANG);
        if (pid == 0)
            return;
        else
        {
            deletePid(backgroundList, pid);
            insertPid(finishedList, pid, processName, jobNumber, false);
        }
        iter = iter->next;
    }
}

void check_foreground_process(struct process **foregroundList, int *jobNumber)
{
    pid_t pid;
    struct process *current = *foregroundList;
    char *processName;
    if (current == NULL)
        return;
    processName = current->processName;
    pid = wait(NULL);
    if (pid == 0)
        return;
    else
    {
        deletePid(foregroundList, pid);
    }
}

char *buildString(char *array[MAX_LINE / 2 + 1])
{
    char *temp = malloc(MAX_LINE / 2 + 1);
    int i=0;
    while (array[i] != NULL)
    {
        temp = concat(temp, array[i]);
        temp = concat(temp, " ");
        i++;
    }
    return temp;
}
