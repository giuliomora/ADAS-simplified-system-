#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "def.h"
#include "util.h"

int main(void)
{
    int fd, len = 0, n;
    char str[1024];

    fd = open(ECU_LOG, O_RDONLY); // Open log file
    if (fd == -1)
    {
        perror("open ecu log");
        exit(EXIT_FAILURE);
    }

    do
    {
        memset(str, 0, sizeof str);
        // Get file string current length
        off_t currentPos = lseek(fd, (size_t)0, SEEK_END);
        readLineFromIndex(fd, str, &len); // Read from position
        if (currentPos > len)
        {
            len = currentPos; // update current position
            printf("%s", str);
        }
    } while (1);
    close(fd);

    return 0;
}