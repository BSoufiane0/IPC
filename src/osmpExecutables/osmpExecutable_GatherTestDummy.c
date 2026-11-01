/******************************************************************************
* FILE: osmpExecutable_GatherTestDummy.c
* DESCRIPTION:
* Program that demonstrates the use of the OSMP_Gather function to collect data from all processes to a single root process
* In this test each process sends a message to the gather
* Autor: Soufiane Baeloul, Yahya Foundou
* Configuration: 50 ./osmpExecutable_GatherTestDummy
******************************************************************************/


#include <stdio.h>

#include "../osmpLibrary/OSMP.h"

int main(int argc, char *argv[]){
    int rank, size;
    OSMP_Init(&argc, &argv);
    OSMP_Rank(&rank);
    OSMP_Size(&size);

    int send_data = rank; //Each process prepares an integer send_data to send. This integer is the rank of the process.
    int recv_data[size]; //Buffer recv_data is prepared to hold the data received by the root process (rank 0). Size of this buffer is the total number of processes.

    //Each process sends 1 integer (OSMP_INT) from send_data. The root process receives these integers into the recv_data buffer.
    OSMP_Gather(&send_data, 1, OSMP_INT, recv_data, 1, OSMP_INT, 0);

    if (rank == 0) { //Root process
        for (int i = 0; i < size; i++) {//Checking if the data received from each process is correct (each process sends its rank)
            if (recv_data[i] != i) {
                printf("Incorrect data received at root from process %d: %d\n", i, recv_data[i]);
            } else {
                printf("Correct data received at root from process %d: %d\n", i, recv_data[i]);
            }
        }
    }

    OSMP_Finalize();
    return 0;

}
