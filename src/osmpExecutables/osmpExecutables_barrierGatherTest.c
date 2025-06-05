//
// Created by kali on 6/1/25.
//
#include "../osmpLibrary/osmpLib.h"



int main(int argc, char *argv[]) {
    int rv, myrank, root=0, gsize, sendarray[20], *rbuf=NULL;
    OSMP_Init(&argc, &argv);


    rv = OSMP_Rank( &myrank );
    // Print a message before the barrier
    printf("Process %d before the barrier\n", myrank);
    if (OSMP_FAILURE == rv) {
        return EXIT_FAILURE;
    }
    if ( myrank == root ) {
        rv = OSMP_Size(&gsize);
        if (OSMP_FAILURE == rv) {
            return EXIT_FAILURE;
        }
        rbuf = (int *)malloc((size_t)gsize*20*sizeof(int));
    }
    rv = OSMP_Gather( sendarray, 20, OSMP_INT, rbuf, 20, OSMP_INT, root);
    if (OSMP_FAILURE == rv) {
        return EXIT_FAILURE;
    }
    return 0;

}