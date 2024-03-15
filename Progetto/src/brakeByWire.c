#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <time.h>

#include "conn.h"
#include "def.h"
#include "util.h"

int logFd;
int clientFd;

void dangerHandler(int sig);
void termHandler(int sig);

int main(int argc, char *argv[])
{
    signal(SIGTERM, termHandler);
    char componentName[] = "brakeByWire";
    clientFd = connectToServer("central"); // Connect to central ECU
    sendComponentName(clientFd, componentName); 

    char str[100], printStr[100];

    logFd = open(BRAKE_LOG, O_WRONLY); // Open log file
    if (logFd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR2, dangerHandler);

    while (1)
    {
        memset(str, 0, sizeof str);
        sendOk(clientFd); // Synchronize central ECU, ready to read
        readLine(clientFd, str); // Read from central ECU

        if (strcmp(str, "FRENO 5") == 0)
        {
            sprintf(printStr, "%d:DECREMENTO 5", (int)time(NULL));
        }

        if (writeln(logFd, printStr) == -1) // Write in log file
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        sleep(1);
    }
    close(logFd);
    close(clientFd);
   
    return 0;
}

/*
    Danger signal handler, listen on SIGUSR2 signal and print ARRESTO AUTO in log file
*/
void dangerHandler(int sig)
{
    char printStr[100];
    sprintf(printStr, "%d:ARRESTO AUTO", (int)time(NULL));

    if (writeln(logFd, printStr) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

/*
    Term signal handler, listen on SIGTERM signal and exit
*/
void termHandler(int sig)
{
    close(logFd);
    close(clientFd);
    exit(EXIT_SUCCESS);
}