#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>

char *concat(const char *, const char *);

char *findPath(char *command)
{
    char *paths; 
    paths = getenv("PATH");
    // şurda size ı düzegün alamadım
    int length = strlen(paths);
    char *temp = malloc(length);
    memcpy(temp,paths,length);
    char *token;

    DIR *d;
    struct dirent *dir;
    bool finded = false;
    while ((token = strtok_r(temp, ":", &temp)))
    {
        d = opendir(token);
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

    char *path = concat("/",command);
    path = concat(token,path);

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

void remove_element(char *array, int index, int array_length)
{
    int i;
    for (i = index; i < array_length - 1; i++)
        array[i] = array[i + 1];
}