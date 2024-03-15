#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include "conn.h"
#include "def.h"
#include "util.h"

void termHandler(int sig);
int clientFd, logFd;

int main(int argc, char *argv[])
{
    signal(SIGTERM, termHandler);
    char componentName[] = "steerByWire";
    clientFd = connectToServer("central"); // Connect to central ECU
    sendComponentName(clientFd, componentName); // Send component info to central ECU
    int flags = fcntl(clientFd, F_GETFL, 0);
    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK); // Set socket non block

    char str[1024], printStr[1024];
    int c, repeat;

    logFd = open(STEER_LOG, O_WRONLY); // Open log file
    if (logFd == -1)
    {
        perror("open steer log");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        memset(str, 0, sizeof str);

        readLine(clientFd, str); // Read central ECU non block
        repeat = 1; // Repeat printing 1 time by default

        if (strcmp(str, "SINISTRA") == 0)
        {
            repeat = 4; // Repeat printing 4 times if turn left
            strcpy(printStr, "STO GIRANDO A SINISTRA");
        }
        else if (strcmp(str, "DESTRA") == 0)
        {
            repeat = 4; // Repeat printing 4 times if turn right
            strcpy(printStr, "STO GIRANDO A DESTRA");
        }
        else
        {
            strcpy(printStr, "NO ACTION");
        }

        for (c = 0; c < repeat; ++c)  // Repeat printing n times
        {
            if (writeln(logFd, printStr) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }

            sleep(1);
        }
    }

    close(clientFd);
    close(logFd);

    return 0;
}

/*
    SIGTERM handler, terminate surround view cameras then exit
*/
void termHandler(int sig)
{
    close(clientFd);
    close(logFd);
    exit(EXIT_SUCCESS);
}