/******************************************************************************
 * FILE: echoall.c
 * AUTHOR: Darius Malysiak
 * DESCRIPTION:
 * First OSMP Executable, it simply echoes all arguments passed to it.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "../osmpLibrary/osmpLib.h"



int main(int argc, char **argv) {
    int rv; //return value
    int size, rank;
    rv = OSMP_Init(&argc, &argv);
    if (OSMP_FAILURE == rv) {
        printf("error at init\n");
        return EXIT_FAILURE;
    }
    sleep(3);
    rv = OSMP_Size(&size);

    if (OSMP_FAILURE == rv) {
        printf("error at size\n");
        return EXIT_FAILURE;
    }
    rv = OSMP_Rank(&rank);
    if (OSMP_FAILURE == rv) {
        printf("error at rank\n");
        return EXIT_FAILURE;
    }
    printf("Hello from process %d of %d\n", rank, size);
    rv = OSMP_Finalize();
    if (OSMP_FAILURE == rv) {
        printf("error at finalize\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
