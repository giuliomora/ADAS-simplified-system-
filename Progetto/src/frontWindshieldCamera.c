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
int clientFd, logFd, cameraFd;

int main(int argc, char *argv[])
{
    signal(SIGTERM, termHandler);
    char componentName[] = "frontWindshieldCamera";
    clientFd = connectToServer("central");
    sendComponentName(clientFd, componentName);

    char buffer[100];

    logFd = open(CAMERA_LOG, O_WRONLY); // Open log file
    if (logFd == -1)
    {
        perror("open camera log");
        exit(EXIT_FAILURE);
    }

    cameraFd = open(FRONT_CAMERA_DATA, O_RDONLY); // Open camera data file
    if (cameraFd == -1)
    {
        perror("open front camera data");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        readLine(cameraFd, buffer); // Read from camera file
        if (writeln(logFd, buffer) == -1) // Write in log file
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        sendC(clientFd, buffer); // Send to central ECU wait confirmation

        sleep(1);
    }

    close(cameraFd);
    close(logFd);

    return 0;
}

/*
    Term signal handler, listen on SIGTERM signal and exit
*/
void termHandler(int sig)
{
    close(logFd);
    close(cameraFd);
    close(clientFd);
    exit(EXIT_SUCCESS);
}