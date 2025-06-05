


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

    if (argc < 3) {
        printf("usage: prog prog2 num_child\n");
        exit(1);
    }

    char** args_null = (char**) malloc((size_t)argc * sizeof(char*));


    if (args_null == NULL) {
        perror("malloc");
    }

    for (int i = 0; i < argc-1; i++) {
        args_null[i] = argv[i+1];
        argv[i+1] = NULL;
    }

    args_null[argc] = NULL;
    for (int i = 0; i < argc-1; i++)
        printf("%s\n", args_null[i]);

    //execv(argv[2], &argv[1]);
    //perror("Fehler bei execvp");
    exit(EXIT_FAILURE);

}

































