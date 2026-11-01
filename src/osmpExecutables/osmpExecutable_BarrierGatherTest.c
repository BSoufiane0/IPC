
/******************************************************************************
* FILE: osmpExecutable_BarrierGatherTest.c
* DESCRIPTION:
* Program that tests OSMP_BARRIERGATHER WITH A LOT N PROCESSES
* Autor:soufiane Baeloul, Yahya Foundou
* Configuration: 50 ./osmpExecutable_BarrierGatherTest
******************************************************************************/


#include <stdio.h>

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
    OSMP_BarrierGather();

// Print a message after the barrier
    printf("Process %d after the barrier\n", rank);
    fflush(stdout);  // Ensure the message is printed immediately

    OSMP_Finalize();
    return 0;

}
