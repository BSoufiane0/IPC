//
// Created by kali on 6/1/25.
//
#include "../osmpLibrary/osmpLib.h"


#define NUM_BARRIERS 10

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