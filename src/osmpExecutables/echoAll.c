/******************************************************************************
 * FILE: echoall.c
 * AUTHOR: Darius Malysiak
 * DESCRIPTION:
 * First OSMP Executable, it simply echoes all arguments passed to it.
 ******************************************************************************/

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#include <time.h>

#define UNUSED(x) (void)(x);



void log_event(const char *event, pid_t pid) {
    FILE *log = fopen("osmp_log.txt", "a");
    if (log == NULL) {
        perror("Logdatei konnte nicht geöffnet werden");
        return;
    }
    time_t now = time(NULL);
    fprintf(log, "[%ld] %s: PID = %d\n", now, event, pid);
    fclose(log);
}

int main(int argc, char **argv) {

    printf("echoall prog \n");

    return EXIT_SUCCESS;
    }

