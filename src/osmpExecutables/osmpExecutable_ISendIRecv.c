/******************************************************************************
* FILE: osmpExecutable_ISendIRecv.c
* DESCRIPTION:
* OSMP programm for testing ISend() and IRecv() with N processes/Threads
 *
* AUTOR : Soufiane Baeloul, Yahya Foundou.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../osmpLibrary/OSMP.h"

int main(int argc, char *argv[]) {
    int size, rank, source, len, errflag=0;
    int bufin[1], bufout[1];
    OSMP_Request *sendrequest = NULL;
    OSMP_Request *recvrequest = NULL;

    OSMP_Init(&argc, &argv);
    OSMP_Size(&size);
    OSMP_Rank(&rank);

    bufin[0] = rank;

    sendrequest = (OSMP_Request *)malloc(sizeof (OSMP_Request) * (size_t) size);
    recvrequest = (OSMP_Request *)malloc(sizeof (OSMP_Request) * (size_t) size);
    for (int i = 0; i < size; ++i) {
        if (rank != i) {
            OSMP_CreateRequest(&sendrequest[i]);
            OSMP_ISend(bufin, 1, OSMP_INT, i, sendrequest[i]);
            OSMP_RemoveRequest( &sendrequest[i]);
        }
    }
    for (int i = 0; i < size-1; ++i) {
        OSMP_CreateRequest( &recvrequest[i]);
        OSMP_IRecv( bufout, 1, OSMP_INT, &source, &len, recvrequest[i]);
        OSMP_Wait( recvrequest[i]);
        // OSMP_IRecv() completed, use bufout
        if(source != bufout[0]) {
            printf("Fehler source %d != bufout %d\n", source, bufout[0]);
            errflag = 1;
        }
        OSMP_RemoveRequest( &recvrequest[i]);
    }

    if(errflag != 1)
        printf("%d alles ok.\n", rank);


    free(sendrequest);
    free(recvrequest);

    OSMP_Finalize();
    return OSMP_SUCCESS;

}
