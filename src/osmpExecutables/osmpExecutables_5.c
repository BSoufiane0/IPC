//
// Created by kali on 6/4/25.
//
#include "../osmpLibrary/osmpLib.h"

#define SIZE 1096

int main(int argc, char *argv[]) {
    printf("start prak5 \n");
    int rv, size, rank, source;
    size_t len;
    char *bufin, *bufout;
    OSMP_Request myrequest ;

    bufin = malloc(SIZE);
    rv = OSMP_Init( &argc, &argv );
    if (OSMP_FAILURE == rv) {
        return EXIT_FAILURE;
    }
    rv = OSMP_Size( &size );
    if (OSMP_FAILURE == rv) {
        return EXIT_FAILURE;
    }
    rv = OSMP_Rank( &rank );
    if (OSMP_FAILURE == rv) {
        return EXIT_FAILURE;
    }
    if( 2 != size ) { /* Fehlerbehandlung */ }
    if( 0 == rank ) { // OSMP process 0 bufin = malloc(SIZE); // check for !=
        len = strlen("hello from 0 rank to the 1 rank through Asynchron library ..."); // length
        memcpy(bufin,"hello from 0 rank to the 1 rank through Asynchron library ...", len);
        rv = OSMP_Send( bufin, (int)len, OSMP_BYTE, 1 );
        if (OSMP_FAILURE == rv) {
            return EXIT_FAILURE;
        }
        printf("rank: %d, sends : %s\n", rank, bufin);
    }
    else { // OSMP process 1
        bufout = calloc(SIZE, sizeof(char)); // check for != NULL
        rv = OSMP_CreateRequest( &myrequest );
        if (OSMP_FAILURE == rv) {
            return EXIT_FAILURE;
        }
        rv = OSMP_IRecv( bufout, SIZE, OSMP_BYTE, &source, (int*)&len, myrequest );
        if (OSMP_FAILURE == rv) {
            return EXIT_FAILURE;
        }
        // check if operation is completed and wait if not
        OSMP_Wait(myrequest);

        printf("rank: %d, receives: %s\n", rank, bufout);

        if (OSMP_FAILURE == rv) {
            return EXIT_FAILURE;
        }
        // OSMP_IRecv() completed, use bufout
        rv = OSMP_RemoveRequest( &myrequest );
        if (OSMP_FAILURE == rv) {
            return EXIT_FAILURE;
        }
    }
    rv = OSMP_Finalize();
    if (OSMP_FAILURE == rv) {
        return EXIT_FAILURE;
    }
    return 0;
}