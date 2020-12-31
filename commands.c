#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "utils.c"

const char *COMMANDS[] = {"ps_all", "search", "bookmark", "exit"};

int check_command(char *command)
{
    if (command == NULL)
        return 0;
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

void check_exit(struct process **background_pids)
{

    struct process *iter = *background_pids;
    if (iter == NULL)
        exit(0);
    else
    {
        printf("There are still running background applications.");
    }
}

void search(char* path,int isRecursive){

printf("\nsearch(%s and %d\n",path,isRecursive);

}


void run_command(int index, struct process **background_pids, struct process **finished_pids, int *jobNumber, char *args[])
{

    switch (index)
    {
    case 1:
        printf("Case 1");
        ps_all(background_pids, finished_pids, jobNumber);
        break;

    case 2:

        printf("Case 2");
        if(strcmp(args[1],"-r") == 0){
            printf("A");
            search(args[2],1);
        }else{
            printf("B");
            search(args[1],0);
        }

        break;

    case 4:
        check_exit(background_pids);
        break;

    default:
        break;
    }
}

void handler(int signo)
{
    if (foreground_pid == NULL)
        return;
    pid_t pid = foreground_pid->pid;
    deletePid(&foreground_pid, pid);
    kill(pid, SIGKILL);
}

void change_signal()
{
    struct sigaction act;
    act.sa_handler = handler; /* set up signal handler */
    act.sa_flags = 0;
    if ((sigemptyset(&act.sa_mask) == -1) ||
        (sigaction(20, &act, NULL) == -1))
    {
        perror("Failed to set SIGSTOP handler");
    }
}
