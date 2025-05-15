
#include <stdio.h>

#include "../osmpLibrary/osmpLib.h"


int main(int argc, char **argv) {
    int rv, size, rank, source, len;
    char bufin[1024], bufout[1024];
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
    if( 2 != size ) {
        // Falsche Anzahl an Prozessen, diese Implementierung ist nur für 2 Prozesse gedacht
        return EXIT_FAILURE;
    }
    /* fill bufin with "ABCDE...ABC..." */
    for( int i = 0; i < 1023; i++) {
        bufin[i] = (char) ('A' + i%26);
    }
    bufin[1023] = '\0';
    if( 0 == rank ) {
        // OSMP process 0
        rv = OSMP_Send( bufin, 1024, OSMP_UNSIGNED_CHAR, 1 );
        if (OSMP_FAILURE == rv) {
            return EXIT_FAILURE;
        }
    } else {
        // OSMP process 1
        rv = OSMP_Recv( bufout, 1024, OSMP_UNSIGNED_CHAR, &source, &len );
        if (OSMP_FAILURE == rv) {
            return EXIT_FAILURE;
        }
        printf("OSMP process %d received %d byte from %d\n%s\n",rank, len, source, bufout);
    }
    rv = OSMP_Finalize();
    if (OSMP_FAILURE == rv) {
        return EXIT_FAILURE;
    }
    return 0;
}