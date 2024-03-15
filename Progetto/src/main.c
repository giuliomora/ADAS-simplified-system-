#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <signal.h>

#include "conn.h"
#include "def.h"
#include "util.h"

struct Component hmiInput, steerByWire, brakeByWire, throttleControl, forwardFacingRadar, frontWindshieldCamera, parkAssist;
unsigned int speed = 0;
int centralFd, logFd;

int createLog(char *path);
int getMode(char *inputString);
void initLogFiles(void);
void removeLogFiles(void);
void execComponents(char *mode);
void centralECU(char *mode);
void execParkAssist(char *mode);
int checkHex(char *buffer);
void bindComponentAndWaitStart(struct Component *components);
void frontWindshieldCameraReader(unsigned int *readSpeed, unsigned int *suspend, unsigned int *parking);
int parkingReader(char *mode);
void slowingDownToParking(unsigned int *brakeByWireReady, char *mode);
void speedUp(unsigned int *throttleControlReady, unsigned int readSpeed);
void slowingDown(unsigned int *brakeByWireReady, unsigned int readSpeed);
void hmiInputReader(unsigned int *parking, unsigned int *suspend);

int main(int argc, char *argv[])
{
    int mode; // launch mode
    mode = getMode(argv[1]);
    initLogFiles();
    execComponents(argv[1]);

    centralFd = initServerSocket("central");
    printf("Start central ecu, waiting for connections...\n");
    struct Component components[7];
    for (int i = 0; i < 6; ++i)
    {
        components[i] = connectToComponent(centralFd);
        int flags = fcntl(components[i].fd, F_GETFL, 0);
        fcntl(components[i].fd, F_SETFL, flags | O_NONBLOCK);
    }
    bindComponentAndWaitStart(components);
    logFd = open(ECU_LOG, O_WRONLY); // Open log file
    if (logFd == -1)
    {
        perror("open ecu log");
        exit(EXIT_FAILURE);
    }
    centralECU(argv[1]);
    close(centralFd);
    close(logFd);
    unlink("central");
    return 0;
}

/*
    Create log file given a path
*/
int createLog(char *path)
{
    int fd = open(path, O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("create log");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return 1;
}

/*
    Validate user input modality
*/
int getMode(char *inputString)
{
    // Check input
    if (inputString == NULL)
    {
        printf("Doesn't exist an argument after Main.\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(NORMAL, inputString) == 0)
    {
        return 0;
    }
    else if (strcmp(ARTIFICIAL, inputString) == 0)
    {
        return 1;
    }
    else
    {
        printf("Modality invalid.\n");
        exit(EXIT_FAILURE);
    }
}

/*
    Remove log files 
*/
void removeLogFiles(void)
{
    remove(ECU_LOG);
    remove(STEER_LOG);
    remove(THROTTLE_LOG);
    remove(BRAKE_LOG);
    remove(CAMERA_LOG);
    remove(RADAR_LOG);
    remove(ASSIST_LOG);
    remove(CAMERAS_LOG);
}

/*
    Init log files
*/
void initLogFiles(void)
{
    removeLogFiles();
    printf("Init log files\n");
    int state = createLog(ECU_LOG) &&
                createLog(STEER_LOG) &&
                createLog(THROTTLE_LOG) &&
                createLog(BRAKE_LOG) &&
                createLog(CAMERA_LOG) &&
                createLog(RADAR_LOG) &&
                createLog(ASSIST_LOG) &&
                createLog(CAMERAS_LOG);
    if (!state)
    {
        removeLogFiles();
        printf("Log files creation failed.\n");
        exit(EXIT_FAILURE);
    }
}

/*
    Execute components in child process
*/
void execComponents(char *mode)
{
    int processes = 6, i;

    for (i = 1; i <= processes; i++)
    {
        if (fork() == 0)
        {
            if (i == 1)
            {
                printf("Start brake by wire\n");
                char *args[] = {BRAKE_BY_WIRE, mode, NULL};
                if (execvp(BRAKE_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 2)
            {
                printf("Start forward facing radar\n");
                char *args[] = {FORWARD_FACING_RADAR, mode, NULL};
                if (execvp(FORWARD_FACING_RADAR, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 3)
            {
                printf("Start front windshield camera\n");
                char *args[] = {FRONT_WINDSHIELD_CAMERA, mode, NULL};
                if (execvp(FRONT_WINDSHIELD_CAMERA, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 4)
            {
                printf("Start hmi input\n");
                char *args[] = {HMI_INPUT, mode, NULL};
                if (execvp(HMI_INPUT, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 5)
            {
                printf("Start steer by wire\n");
                char *args[] = {STEER_BY_WIRE, mode, NULL};
                if (execvp(STEER_BY_WIRE, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if (i == 6)
            {
                printf("Start throttle control\n");
                char *args[] = {THROTTLE_CONTROL, mode, NULL};
                if (execvp(THROTTLE_CONTROL, args) < 0)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

/*
    Execute park assist in child process
*/
void execParkAssist(char *mode)
{
    if (fork() == 0)
    {
        printf("Start park assist\n");
        char *args[] = {PARK_ASSIST, mode, NULL};
        if (execvp(PARK_ASSIST, args) < 0)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
}

/*
    Throttle control signal SIGUSR2 Handler, stop all process and exit
*/
void throttleControlBrokeHandler(int sig)
{
    printf("Throttle control rotto!\n");
    speed = 0;
    kill(hmiInput.pid, SIGTERM);
    kill(throttleControl.pid, SIGTERM);
    kill(brakeByWire.pid, SIGTERM);
    kill(steerByWire.pid, SIGTERM);
    kill(frontWindshieldCamera.pid, SIGTERM);
    kill(forwardFacingRadar.pid, SIGTERM);
    close(centralFd);
    close(logFd);
    unlink("central");
    exit(EXIT_SUCCESS);
}

/*
    Bind components to global variables, wait user input start
*/
void bindComponentAndWaitStart(struct Component *components)
{
    for (int i = 0; i < 6; ++i)
    {
        if (strcmp(components[i].name, "hmiInput") == 0)
        {
            hmiInput = components[i];
            sendOk(hmiInput.fd); // Start listen user input
            while (1)
            {
                memset(hmiInput.buffer, 0, sizeof hmiInput.buffer);
                if (readLine(hmiInput.fd, hmiInput.buffer) > 0)
                {
                    sendOk(hmiInput.fd);
                    if (strcmp(hmiInput.buffer, "INIZIO") == 0)
                    {
                        break;
                    }
                }
                sleep(1);
            }
        }
        if (strcmp(components[i].name, "steerByWire") == 0)
        {
            steerByWire = components[i];
        }
        if (strcmp(components[i].name, "brakeByWire") == 0)
        {
            brakeByWire = components[i];
        }
        if (strcmp(components[i].name, "forwardFacingRadar") == 0)
        {
            forwardFacingRadar = components[i];
        }
        if (strcmp(components[i].name, "frontWindshieldCamera") == 0)
        {
            frontWindshieldCamera = components[i];
        }
        if (strcmp(components[i].name, "throttleControl") == 0)
        {
            throttleControl = components[i];
        }
    }
}

/*
    Central ECU logic
*/
void centralECU(char *mode)
{
    unsigned int readSpeed = 0;             // Read speed limit
    unsigned int throttleControlReady = 0;  // Throttle control is ready, so don't need to read from it's fd
    unsigned int brakeByWireReady = 0;      // Brake by wire is ready, so don't need to read from it's fd
    unsigned int suspend = 0;               // Read PERICOLO, suspend central ECU until user input INIZIO
    unsigned int parking = 0;               // Read PARCHEGGIO from front windshield camera or from user input

    signal(SIGUSR1, throttleControlBrokeHandler); // Set speed to 0 and terminate

    while (1)
    {
        memset(hmiInput.buffer, 0, sizeof hmiInput.buffer);
        if (!parking && readLine(hmiInput.fd, hmiInput.buffer) > 0) // Ignore HMI info if parking is requested
        {
            hmiInputReader(&parking, &suspend);
        }

        memset(forwardFacingRadar.buffer, 0, sizeof forwardFacingRadar.buffer);        
        if (!parking && readLine(forwardFacingRadar.fd, forwardFacingRadar.buffer) > 0) // Ignore forward facing radar info if parking is requested
        {
            sendOk(forwardFacingRadar.fd);
        }

        // Read speed to be achieved, turn left, turn right and danger from front windshield camera
        memset(frontWindshieldCamera.buffer, 0, sizeof frontWindshieldCamera.buffer);               
        if (!parking && !suspend && readLine(frontWindshieldCamera.fd, frontWindshieldCamera.buffer) > 0) // Ignore front windshield camera if suspend or parking
        {
            frontWindshieldCameraReader(&readSpeed, &suspend, &parking);
        }

        if (!suspend && !parking) // Ignore actuators if suspend
        {
            speedUp(&throttleControlReady, readSpeed);
            slowingDown(&brakeByWireReady, readSpeed);
        }

        if (parking)
        {
            memset(brakeByWire.buffer, 0, sizeof brakeByWire.buffer); 
            if (speed >= 5 && (brakeByWireReady || readLine(brakeByWire.fd, brakeByWire.buffer) > 0)) // Brake by wire ready to update
            {
                slowingDownToParking(&brakeByWireReady, mode);
            }

            if (speed == 0 && parkingReader(mode)) // Ready to parking if park success exit from while
            {
                break;
            }
        }

        usleep(100000); // 0.1s to use less cpu, central ECU should be always active
    }
}

/*
    Check if the string given contains special sequence of char
*/
int checkHex(char *buffer)
{
    return strstr(buffer, "172A") != NULL || strstr(buffer, "D693") != NULL || strstr(buffer, "0000") != NULL || strstr(buffer, "BDD8") != NULL || strstr(buffer, "FAEE") != NULL || strstr(buffer, "4300") != NULL;
}

/*
    Parking component reader
*/
int parkingReader(char *mode)
{
    memset(parkAssist.buffer, 0, sizeof parkAssist.buffer); // Reset buffer
    if (readLine(parkAssist.fd, parkAssist.buffer) > 0)     // Brake by wire ready to update
    {
        if (checkHex(parkAssist.buffer))
        {
            printf("PARKING FAILED, RESTART PARKING\n");
            kill(parkAssist.pid, SIGTERM); // Stop park assist and restart
            execParkAssist(mode);
            parkAssist = connectToComponent(centralFd);
            int flags = fcntl(parkAssist.fd, F_GETFL, 0);
            fcntl(parkAssist.fd, F_SETFL, flags | O_NONBLOCK);
        }
        else if (strcmp(parkAssist.buffer, "END") == 0) // Parking assist finished
        {
            kill(hmiInput.pid, SIGTERM); // Park assist and surround view cameras should quit by their self
            return 1;
        }
    }
    return 0;
}

/*
    Front windshield camera component reader
*/
void frontWindshieldCameraReader(unsigned int *readSpeed, unsigned int *suspend, unsigned int *parking)
{
    sendOk(frontWindshieldCamera.fd); // Sync

    if (isNumber(frontWindshieldCamera.buffer)) // Read a number
    {
        *readSpeed = toNumber(frontWindshieldCamera.buffer);
    }

    if (strcmp(frontWindshieldCamera.buffer, "DESTRA") == 0 || strcmp(frontWindshieldCamera.buffer, "SINISTRA") == 0)
    {
        writeln(logFd, frontWindshieldCamera.buffer);
        sendMsg(steerByWire.fd, frontWindshieldCamera.buffer); // Send command to steer by wire
    }

    if (strcmp(frontWindshieldCamera.buffer, "PERICOLO") == 0)
    {
        writeln(logFd, "ARRESTO");
        kill(brakeByWire.pid, SIGUSR2); // Send signal to brake by wire
        *suspend = 1;                    // Suspend until user input INIZIO
        speed = 0;
    }

    if (strcmp(frontWindshieldCamera.buffer, "PARCHEGGIO") == 0)
    {
        writeln(logFd, "PARCHEGGIO");
        *parking = 1;
    }
}

/*
    Slow down to parking if parking is active
*/
void slowingDownToParking(unsigned int *brakeByWireReady, char *mode)
{
    *brakeByWireReady = 0;
    speed -= 5;
    writeln(logFd, "FRENO 5");
    sendMsg(brakeByWire.fd, "FRENO 5");

    if (speed == 0) // Ready to parking
    {
        execParkAssist(mode);
        parkAssist = connectToComponent(centralFd);
        int flags = fcntl(parkAssist.fd, F_GETFL, 0);
        fcntl(parkAssist.fd, F_SETFL, flags | O_NONBLOCK);

        // Terminate sensors and actuators
        kill(frontWindshieldCamera.pid, SIGTERM);
        kill(forwardFacingRadar.pid, SIGTERM);
        kill(steerByWire.pid, SIGTERM);
        kill(brakeByWire.pid, SIGTERM);
        kill(throttleControl.pid, SIGTERM);
    }
}

/*
    Speed up, check if throttle control is ready
*/
void speedUp(unsigned int *throttleControlReady, unsigned int readSpeed)
{
    memset(throttleControl.buffer, 0, sizeof throttleControl.buffer);                     // Reset buffer
    if (*throttleControlReady || readLine(throttleControl.fd, throttleControl.buffer) > 0) // Throttle control ready to update
    {
        *throttleControlReady = 1;
        if (speed < readSpeed)
        {
            speed += 5;
            writeln(logFd, "INCREMENTO 5");
            sendMsg(throttleControl.fd, "INCREMENTO 5"); // Send to throttle control command
            *throttleControlReady = 0;
        }
    }
}

/*
    Slowing down, check if brake by wire is ready
*/
void slowingDown(unsigned int *brakeByWireReady, unsigned int readSpeed)
{
    memset(brakeByWire.buffer, 0, sizeof brakeByWire.buffer);
    if (*brakeByWireReady || readLine(brakeByWire.fd, brakeByWire.buffer) > 0) // Brake by wire ready to update
    {
        *brakeByWireReady = 1;
        if (speed > readSpeed)
        {
            speed -= 5;
            writeln(logFd, "FRENO 5");
            sendMsg(brakeByWire.fd, "FRENO 5"); // Send to brake by wire command
            *brakeByWireReady = 0;
        }
    }
}

/*
    hmi input, check if brake by wire is ready
*/
void hmiInputReader(unsigned int *parking, unsigned int *suspend)
{
    if (strcmp(hmiInput.buffer, "PARCHEGGIO") == 0)
    {
        *parking = 1;
    }

    if (strcmp(hmiInput.buffer, "ARRESTO") == 0)
    {
        writeln(logFd, "ARRESTO");
        kill(brakeByWire.pid, SIGUSR2); // Send signal to brake by wire
        speed = 0;
    }

    if (strcmp(hmiInput.buffer, "INIZIO") == 0)
    {
        *suspend = 0; // Restart from status danger
    }

    sendOk(hmiInput.fd); // wake up hmi input
}