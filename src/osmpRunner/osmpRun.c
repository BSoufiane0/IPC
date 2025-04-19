

/******************************************************************************
 * FILE: osmpRun.c
 * AUTHOR: Darius Malysiak
 * DESCRIPTION:
 * Diese Datei enthält die main Funktion für den OSMP-Runner.
 ******************************************************************************/

#include "osmpRun.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>



int main(int argc, char *argv[]) {
    // avoid zombies ...
    signal(SIGCHLD, &sig_child_handler);

    int rv = OSMP_Init(&argc, &argv);
    if (OSMP_FAILURE == rv) {
        return EXIT_FAILURE;
    }

    printf("%d\n", argc);
    char** args_null = (char**) calloc((size_t)argc, sizeof(char*));

    if (args_null == NULL) {
        perror("malloc");
        exit(1);
    }
    //
    for (int i = 0; i < argc-1; i++) {
        args_null[i] = argv[i+1];
        argv[i+1] = NULL;
    }

    args_null[argc-1] = NULL;

    execv(args_null[0], args_null);
    perror("Fehler bei execvp");
    exit(EXIT_FAILURE);

}




