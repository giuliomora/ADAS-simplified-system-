#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include "util.h"
#include "def.h"

/*
    Read file from index, given file descriptor, buffer, index returns read status
*/
void readLineFromIndex(int fd, char *str, int *index)
{
    int n;
    lseek(fd, *index, SEEK_SET);
    do
    {
        n = read(fd, str, 1);
        if (n < 0)
        {
            perror("read line from index");
            exit(EXIT_FAILURE);
        }
    } while (n > 0 && *str++ != '\0');
}

/*
    Write in file with newline, given file descriptor and buffer returns write status.
*/
int writeln(int fd, char *str)
{
    size_t len = strlen(str);
    char *newstr = malloc(len + 2);
    strcpy(newstr, str);
    strcat(newstr, "\n");

    return write(fd, newstr, len + 1);
}

/*
    Read file until read newline, end string symbol, given file descriptor and buffer returns read status.
*/
int readLine(int fd, char *str)
{
    int n;
    while (1)
    {
        n = read(fd, str, 1);
        if (n <= 0 || *str == 10 || *str == '\n' || *str == '\0')
        {
            *str = '\0';
            break;
        }
        str++;
    }

    return n > 0;
}

/*
    Get urandom path, given user input modality
*/
char* getDataSrcUrandom(char *mode)
{
    return strcmp(mode, "ARTIFICIALE") == 0 ? URANDOM_ARTIFICIAL : URANDOM;
}

/*
    Get random path, given user input modality
*/
char *getDataSrcRandom(char *mode)
{
    return strcmp(mode, "ARTIFICIALE") == 0 ? RANDOM_ARTIFICIAL : RANDOM;
}

/*
    Read 8 byte from file convert it in hexadecimal string, given file descriptor and buffer, returns number of byte read
*/
int read8(int fd, char *str)
{
    unsigned long int buffer[8];
    ssize_t n = 0;
    n = read(fd, buffer, 8);
    if (n < 0)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    sprintf(str, "%016lX", *buffer);

    return n;
}

/*
    Check if string is a valid number, returns 1 o 0
*/
int isNumber(char *str)
{
    for (int i = 0, len = strlen(str); i < len; i++)
    {
        if (!isdigit(str[i]))
        {
            return 0;
        }
    }

    return 1;
}

/*
    Convert string into number, given a string, returns a number
*/
int toNumber(char *str)
{
    int num = 0;

    for (int i = 0; str[i] != '\0'; i++)
    {
        num = num * 10 + (str[i] - 48);
    }

    return num;
}