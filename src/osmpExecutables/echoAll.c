/******************************************************************************
 * FILE: echoall.c
 * AUTHOR: Darius Malysiak
 * DESCRIPTION:
 * First OSMP Executable, it simply echoes all arguments passed to it.
 ******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "../osmpLibrary/osmpLib.h"


#define UNUSED(x) (void)(x);

int main(int argc, char **argv) {

    int size, rank, rv, i;
    printf("========== PID = %d ==============\n", _getpid());
    rv = OSMP_Init(&argc, &argv);

    if (OSMP_FAILURE == rv) {
        printf("error in OSMP_Init\n");
        return EXIT_FAILURE;
    }

    rv = OSMP_Rank(&rank);
    if (OSMP_FAILURE == rv) {
        printf("rank error\n");
        return EXIT_FAILURE;
    }
    printf("parent Rank = %d.\n", rank);

    for ( i = 0; i < pid_pool->total_size-1; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            break;
        } else if (pid == 0) {
            // Child process
            rv = OSMP_Init(&argc, &argv);
            if (OSMP_FAILURE == rv) {
                printf("init error\n");
                return EXIT_FAILURE;
            }
            rv = OSMP_Size(&size);
            if (OSMP_FAILURE == rv) {
                printf("size error\n");
                return EXIT_FAILURE;
            }
            rv = OSMP_Rank(&rank);
            if (OSMP_FAILURE == rv) {
                printf("rank error\n");
                return EXIT_FAILURE;
            }
            printf("Hello from process %d of %d\n", rank, size);
            for (int j = 0; j < pid_pool->total_size; j++)
                printf("ps %d: %d \n", j, pid_pool->pool[j]);

            sleep(3);

            rv = OSMP_Finalize();
            if (OSMP_FAILURE == rv) {
                printf("finalize error\n");
                return EXIT_FAILURE;
            }
            exit(EXIT_SUCCESS);

        }
        printf("child: %d is forked \n", pid);
        printf("parent: pid_pool_meta_data: %d, %d, %d \n",pid_pool->is_set, pid_pool->len, pid_pool->total_size);

    }
    printf("continue parent ...\n");



    // wait until all children are done ...
    while (pid_pool->len > 1);


    rv = OSMP_Finalize();
    if (OSMP_FAILURE == rv) {
        return EXIT_FAILURE;
    }

	return EXIT_SUCCESS;
}
