/******************************************************************************
* FILE: osmpExecutable_BarrierTest2.c
* DESCRIPTION:
* Program test of OSMP_BARRIER: CALL BARRIER 10000 TIMES
* Autor: Soufiane Baeloul, Yahya Foundou
* Configuration: 10 ./osmpExecutable_BarrierTest2
******************************************************************************/


#include <stdio.h>

#include "../osmpLibrary/OSMP.h"

#define NUM_BARRIERS 10000

int main(int argc, char *argv[]){
    int rank;
    OSMP_Init(&argc, &argv);
    OSMP_Rank(&rank);
    for (int i = 0; i < NUM_BARRIERS; i++) {
        OSMP_Barrier(); // Calling the osmp_barrier 1000 times
        if(rank == 0){ //Calling a proc and printing a message after each barrier to make sure that the barrier was called 1000 times.
            printf("Durchlauf Nr.%d fertig\n", i);
        }
    }

    OSMP_Finalize();
    return 0;

}
