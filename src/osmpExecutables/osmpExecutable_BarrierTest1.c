/******************************************************************************
* FILE: osmpExecutable_BarrierTest1.c
* DESCRIPTION:
* Program that tests OSMP_BARRIER WITH A LOT N PROCESSES
* Autor: Soufiane Baeloul, Yahya Foundou
* Configuration: 50 ./osmpExecutable_BarrierTest1
******************************************************************************/


#include <stdio.h>

#include <unistd.h> // For sleep function

#include "../osmpLibrary/OSMP.h"

int main(int argc, char *argv[]) {
    int rank, size;
    OSMP_Init(&argc, &argv);
    OSMP_Rank(&rank);
    OSMP_Size(&size);
// Print a message before the barrier
    printf("Process %d before the barrier\n", rank);
    fflush(stdout);  // Ensure the message is printed immediately

// Synchronize all processes at this point
    OSMP_Barrier();

// Print a message after the barrier
    printf("Process %d after the barrier\n", rank);
    fflush(stdout);  // Ensure the message is printed immediately

    OSMP_Finalize();
    return 0;

}
