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

int clientFd;
void termHandler(int sig);

int main(void)
{
    signal(SIGTERM, termHandler);
    char componentName[] = "hmiInput";
    clientFd = connectToServer("central"); // Connect to central ECU
    sendComponentName(clientFd, componentName);

    printf("Waiting for components initialize: ");

    waitOk(clientFd); // Wait all components to initialize
    printf("Done\n");

    char input[1024];
    do
    {
        memset(input, 0, sizeof(input));
        printf("Enter a command: ");

        if (scanf("%s", input) == EOF) // Read input from keyboard
        {
            break;
        }

        if (strcmp(input, "INIZIO") == 0 || strcmp(input, "PARCHEGGIO") == 0 || strcmp(input, "ARRESTO") == 0)
        {
            size_t len = strlen(input);
            sendC(clientFd, input); // Send input to central ECU, wait confirmation
        }
        else
            printf("Invalid command\n");
    } while (1);

    close(clientFd);

    return 0;
}

/*
    Term signal handler, listen on SIGTERM signal and exit
*/
void termHandler(int sig)
{
    close(clientFd);
    exit(EXIT_SUCCESS);
}