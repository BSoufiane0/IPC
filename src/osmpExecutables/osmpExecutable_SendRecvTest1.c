/******************************************************************************
* FILE: osmpExecutable_SendRecvTest1.c
* DESCRIPTION:
* OSMP program test the send of 20 messages from first process 0 to each one of the other N processes
*Configuration: 20 ./osmpExecutable_SendRecvTest1
******************************************************************************/
#include <stdio.h>


#include "../osmpLibrary/OSMP.h"

int main(int argc, char *argv[]) {
    int size, rank, source;
    int bufin[1], bufout[1], len;

    OSMP_Init(&argc, &argv);

    OSMP_Size(&size);

    OSMP_Rank(&rank);

    if (rank == 0) {
        // OSMP process 0 sends 20 message to each one of the other processes
        for (int i = 0; i < 20; ++i) {
            bufin[0] = i + 1;
            for (int j = 0; j < size; ++j) {
                OSMP_Send(bufin, 1, OSMP_INT, j);
            }
        }
    } else {
        // other OSMP processes
        for (int i = 0; i < 20; ++i) {
            OSMP_Recv(bufout, 1, OSMP_INT, &source, &len);
            printf("OSMP process %d received %d byte from %d [%d] \n", rank, len, source, bufout[0]);
        }
    }

    OSMP_Finalize();

    return 0;
}
