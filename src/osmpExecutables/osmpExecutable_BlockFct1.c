/******************************************************************************
* FILE: osmpExecutable_BlockFct1.c
* DESCRIPTION:
* Program that tests to send of N-1 messages from one Process to all other N Processes
* Autor: Soufiane Baeloul, Yahya Foundou
*Configuration: 16 ./osmpExecutable_BlockFct1
******************************************************************************/
#include <stdio.h>


#include "../osmpLibrary/OSMP.h"



int main(int argc, char *argv[]) {
    int size, rank, source;
    int bufin[1], bufout[1], len;
    OSMP_Init(&argc, &argv);
    OSMP_Size(&size);
    OSMP_Rank(&rank);

  //  printf("Process %d initialized\n", rank);

    for (int i = 0; i < size; ++i) {
        bufin[0] = rank;
        if (rank != i) {
           // printf("Process %d sending to process %d\n", rank, i);
            OSMP_Send(bufin, 1, OSMP_INT, i);
        }
    }

    for (int i = 0; i < size-1; ++i) {
        OSMP_Recv(bufout, 1, OSMP_INT, &source, &len);
      //  printf("Process %d received message from process %d with content %d\n", rank, source, bufout[0]);
        if (bufout[0] == source) {
            printf("Process %d received correct message from process %d\n", rank, source);
        }
    }

    OSMP_Finalize();
    return 0;
}
