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
#include <limits.h>
#include <signal.h>

#include "conn.h"
#include "def.h"
#include "util.h"

int clientFd, logFd, randomFd;

void termHandler(int sig);
int throttleBreaks(int fd);

int main(int argc, char *argv[])
{
    char componentName[] = "throttleControl";
    clientFd = connectToServer("central");
    sendComponentName(clientFd, componentName); // Send component info to central ECU

    char str[100], printStr[100];

    logFd = open(THROTTLE_LOG, O_WRONLY); // Open log file
    if (logFd == -1)
    {
        perror("open throttle log");
        exit(EXIT_FAILURE);
    }
    randomFd = open(getDataSrcRandom(argv[1]), O_RDONLY); // Open random file
    if (randomFd == -1)
    {
        perror("open urandom");
        exit(EXIT_FAILURE);
    }

    pid_t ppid;
    ppid = getppid(); // Get parent pid

    while (1)
    {
        memset(str, 0, sizeof str);
        sendOk(clientFd); // Synchronize central ECU
        readLine(clientFd, str);

        if (throttleBreaks(randomFd))
        {
            kill(ppid, SIGUSR1); // Send SIGUSR1 to central ECU if throttle control break
            break;
        }

        if (strcmp(str, "INCREMENTO 5") == 0)
        {
            sprintf(printStr, "%d:INCREMENTO 5", (int)time(NULL));

            if (writeln(logFd, printStr) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }

        sleep(1);
    }
    close(logFd);
    close(randomFd);
    close(clientFd);
   
    return 0;
}

/*
    Simulate probability of 1/10^-5 read a random number from file "random "returns 1 if throttle breaks else returns 0
*/
int throttleBreaks(int fd)
{
    unsigned int randomNumber;
    int result = read(fd, &randomNumber, sizeof randomNumber); // Random number is between 0 and 4 294 967 295
    if (result < 0)
    {
        perror("read random");
        exit(EXIT_FAILURE);
    }
    double range = (double)UINT_MAX / 100000; // range is 42949.67295
    return randomNumber < range;  // if the random number is less then 42949 then the probabiltiy is 1/10^-5
}

/*
    SIGTERM handler, terminate surround view cameras then exit
*/
void termHandler(int sig)
{
    close(clientFd);
    close(logFd);
    close(randomFd);
    exit(EXIT_SUCCESS);
}