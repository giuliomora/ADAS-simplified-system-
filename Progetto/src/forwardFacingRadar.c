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
int clientFd, logFd, urandomFd;

int main(int argc, char *argv[])
{
    signal(SIGTERM, termHandler);
    char componentName[] = "forwardFacingRadar";
    clientFd = connectToServer("central");
    sendComponentName(clientFd, componentName); // Connect to central ECU

    char buffer[1024];

    logFd = open(RADAR_LOG, O_WRONLY); // Open log file
    if (logFd == -1)
    {
        perror("open radar log");
        exit(EXIT_FAILURE);
    }

    urandomFd = open(getDataSrcUrandom(argv[1]), O_RDONLY); // Open file "urandom"
    if (urandomFd == -1)
    {
        perror("open urandom");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        memset(buffer, 0, sizeof buffer);

        int byteRead = read8(urandomFd, buffer); // Read 8 bytes from file "urandom"
        if (byteRead == 8)
        {
            if (writeln(logFd, buffer) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
            sendC(clientFd, buffer); // Send read bytes to central ECU and wait confirmation
        }
        sleep(1);
    }

    close(logFd);
    close(urandomFd);
    close(clientFd);

    return 0;
}

/*
    Term signal handler, listen on SIGTERM signal and exit
*/
void termHandler(int sig)
{
    close(logFd);
    close(urandomFd);
    close(clientFd);
    exit(EXIT_SUCCESS);
}