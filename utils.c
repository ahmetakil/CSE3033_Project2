#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LINE 80

char *concat(const char *, const char *);

struct process
{
    pid_t pid;
    char *processName;
    int jobNumber;
    struct process *next;
};
typedef struct process process;

char *findPath(char *command)
{
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

char *concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
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

void insertPid(struct process **processList, pid_t pid, char *processName, int *jobNumber)
{
    struct process *new = malloc(sizeof(struct process));
    new->pid = pid;
    new->processName = processName;
    new->jobNumber = *jobNumber;

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
    *jobNumber += 1;
}

void deletePid(struct process **processList, pid_t pid, int *jobNumber)
{
    *jobNumber -= 1;
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
