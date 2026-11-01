/******************************************************************************
* FILE: osmpExecutable_GatherTest.c
* DESCRIPTION:
* Program test of OSMP_GATHER: while each process sends N messages declared in macro NUM_MESSAGES to gather
* Autor: Soufiane Baeloul, Yahya Foundou
* Configuration: 30 ./osmpExecutable_GatherTest
******************************************************************************/

#include <stdio.h>
#include "../osmpLibrary/OSMP.h"

#define NUM_MESSAGES 50


int main(int argc, char *argv[]){
    int rank, size, recv;
    OSMP_Init(&argc, &argv);
    OSMP_Rank(&rank);
    OSMP_Size(&size);

    int send_data[NUM_MESSAGES]; //Array send_data to hold the data (NUM_MESSAGES) that each process will send.
    int recv_data[size * NUM_MESSAGES]; //A buffer recv_data to hold all the data received by the root process (rank 0).size of buffer is size(nr proc) * nr messages pro proc

// Initialize send_data with the rank of the process
    for (int i = 0; i < NUM_MESSAGES; i++) {
        send_data[i] = rank; //each process will send an array filled with its rank.
    }

// Perform the gather operation with 50 messages per process
//This function gathers data from all processes to the root process (rank 0). Each process sends NUM_MESSAGES integers (OSMP_INT) from send_data.
// The root process receives these integers into the recv_data buffer.
   recv= OSMP_Gather(send_data, NUM_MESSAGES, OSMP_INT, recv_data, NUM_MESSAGES, OSMP_INT, 0);
    if(recv != OSMP_SUCCESS){
        perror("OSMP_Gather Failure \n");
    }

    if (rank == 0) { //Root process
        for (int i = 0; i < size * NUM_MESSAGES; i++) {//root process iterates over the recv_data buffer and checks if the data received from each process is correct.
            if (recv_data[i] != (i / NUM_MESSAGES)) { //Checks the rank of the sending process. (use operation divide (i/num msg) because before we used multiplication size*num msg)
                printf("Incorrect data received at root from process %d: %d\n", i / NUM_MESSAGES, recv_data[i]);// TODO EXPLAIN I/NUM MESS
            } else {
                printf("Correct data received at root from process %d: %d\n", i / NUM_MESSAGES, recv_data[i]);// TODO EXPLAIN I/NUM MESS
            }
        }
    }

    OSMP_Finalize();
    return 0;

}
