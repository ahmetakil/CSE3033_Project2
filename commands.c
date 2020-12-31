#include <stdio.h>
#include <string.h>
#include "utils.c"

const char *COMMANDS[] = {"ps_all", "search", "bookmark", "exit"};

int check_command(char *command)
{
    int len = sizeof(COMMANDS) / sizeof(*COMMANDS);
    int i;

    for (i = 0; i < len; i++)
    {
        if (strcmp(COMMANDS[i], command) == 0)
            return i + 1;
    }
    return 0;
}

void ps_all(struct process **background, struct process **finished, int *jobNumber)
{
    printf("Running:\n");
    struct process *iterback = *background;
    while (iterback != NULL)
    {
        printf("\t[%d] %s (Pid=%d)\n", iterback->jobNumber, iterback->processName, (int)iterback->pid);
        iterback = iterback->next;
    }
    if (*background == NULL)
        *jobNumber = 1;
    printf("Finished:\n");
    struct process *iterfin = *finished;
    while (iterfin != NULL)
    {
        printf("\t[%d] %s\n", iterfin->jobNumber, iterfin->processName);
        iterfin = iterfin->next;
    }
    deleteList(finished);
}

void check_background(struct process **backgroundList, struct process **finishedList, int *jobNumber)
{
    pid_t result;
    struct process *iter = *backgroundList;
    if (iter == NULL)
        return;
    char *processName;
    while (iter != NULL)
    {
        processName = iter->processName;
        result = waitpid(iter->pid, NULL, WNOHANG);
        if (result == 0)
            return;
        else
        {
            deletePid(backgroundList, result, jobNumber);
            insertPid(finishedList, result, processName, jobNumber);
        }
        iter = iter->next;
    }
}

void run_command(int index, struct process **background_pids, struct process **finished_pids, int *jobNumber)
{
    switch (index)
    {
    case 1:
        ps_all(background_pids, finished_pids, jobNumber);
        break;

    default:
        break;
    }
}
