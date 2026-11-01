/******************************************************************************
* FILE: osmpExecutable_SendRecvTest2.c
* DESCRIPTION:
* Program that tests (pair process) the send and receive of a message from one Process to the other Process
* Autor: Sanae Sandi ,ILham Asnina
* Configuration: 2 ./osmpExecutable_SendRecvTest2
******************************************************************************/

#include <stdio.h>

#include "../osmpLibrary/OSMP.h"

int main(int argc, char *argv[]){

    int size, rank, source;
    int bufin[2], bufout[2], len;

    // Initialisierung der Prozesse
    if (OSMP_Init(&argc, &argv) == OSMP_FAILURE) {
        printf("osmpexecutable OSMP_Init Fehler\n");
        return OSMP_FAILURE;
    }
    // Größe ermitteln
    size = -1;
    if (OSMP_Size(&size) == OSMP_FAILURE) {
        printf("osmpexecutable OSMP_Size Fehler\n");
        return OSMP_FAILURE;
    }
    // Rang ermitteln
    rank = -1;
    if (OSMP_Rank(&rank) == OSMP_FAILURE) {
        printf("osmpexecutable OSMP_Rank Fehler\n");
        return OSMP_FAILURE;
    }

    if( rank == 0 )
    { // OSMP process 0
        bufin[0] = 4711;
        bufin[1] = 4712;
        OSMP_Send( bufin, 2, OSMP_INT, 1 );
        OSMP_Recv( bufout, 2, OSMP_INT, &source, &len );
        printf("OSMP process %d received %d byte from %d [%d:%d] \n",rank, len, source, bufout[0], bufout[1]);
    }
    else
    { // OSMP process 1
        OSMP_Recv( bufout, 2, OSMP_INT, &source, &len );
        printf("OSMP process %d received %d byte from %d [%d:%d] \n",rank, len, source, bufout[0], bufout[1]);
        OSMP_Send(bufout, 2, OSMP_INT, (rank+1) % size);
    }

    OSMP_Finalize();

    return OSMP_SUCCESS;
}
