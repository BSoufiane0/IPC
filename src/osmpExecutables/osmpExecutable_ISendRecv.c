/******************************************************************************
* FILE: osmpExecutable_ISendRecv.c
* DESCRIPTION:
* OSMP program that calls OSMP_ISend/OSMP_Recv with N processes/Threads
*
* AUTOR : Soufiane Baeloul, Yahya Foundou.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../osmpLibrary/OSMP.h"

int main(int argc, char *argv[]) {
    int size, rank, source, len, flag,  SIZE = 100;
    char *bufin, *bufout;
    OSMP_Request myrequest = NULL;
    OSMP_Init(&argc, &argv);
    OSMP_Size(&size);
    OSMP_Rank(&rank);

    // Loop for non-blocking send
    for (int i = 0; i < size; ++i) {
        if (rank != i) {
            bufin = malloc((size_t) SIZE); // check for != NULL
            len = 12; // length
            memcpy(bufin, "Hello World", (size_t) len);
            OSMP_CreateRequest(&myrequest);
            OSMP_ISend(bufin, SIZE, OSMP_BYTE, i, myrequest);
            //Check if the request is done, delete if true
            OSMP_Test(myrequest, &flag);
            if (flag == OSMP_FINISHED) {
                printf("Isend request complete\n");
                OSMP_RemoveRequest(&myrequest);
                free(bufin);

            } else {
                OSMP_Wait(myrequest);
                OSMP_RemoveRequest(&myrequest);
                free(bufin);
            }
        }
    }


    for (int i = 0; i < size - 1; ++i) {
        bufout = malloc((size_t) SIZE); // check for != NULL
        OSMP_Recv(bufout, SIZE, OSMP_BYTE, &source, &len);
        printf("%s\n", bufout);
        free(bufout);
    }

    OSMP_Finalize();
    return 0;
}
