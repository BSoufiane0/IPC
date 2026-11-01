/******************************************************************************
* FILE: osmpExecutable_SendRecv.c
* DESCRIPTION:
* OSMP program with a simple pair of blocking OSMP_Send/OSMP_Recv calls
* Configuration: 2 ./osmpExecutable_SendRecv
* LAST MODIFICATION: Darius Malysiak
******************************************************************************/
#include <stdio.h>

#include "../osmpLibrary/OSMP.h"


/**
 * this program tests the send & recv from only two sub processes, remember you have to start the
 * runner with 2 processes to fork, otherwise you will have unfinitly wainting .
 */
int main(int argc, char *argv[]) {
   // UNUSED(argc);
   // UNUSED(argv);
    int rv, size, rank, source;
    int bufin[2], bufout[2], len;
    rv = OSMP_Init(&argc, &argv);
    rv = OSMP_Size(&size);
    rv = OSMP_Rank(&rank);
    if (size != 2) {
        /* Fehlerbehandlung */
    }
    if (rank == 0) { 
        // OSMP process 0
        bufin[0] = 4711;
        bufin[1] = 4712;
        rv = OSMP_Send(bufin, 2, OSMP_INT, 1);
    } else { 
        // OSMP process 1
        rv = OSMP_Recv(bufout, 2, OSMP_INT, &source, &len);
        printf("OSMP process %d received %d byte from %d [%d:%d] \n", rank, len, source, bufout[0], bufout[1]);
    }
    rv = OSMP_Finalize();
    printf("%d\n", rv);
    return 0;
}
