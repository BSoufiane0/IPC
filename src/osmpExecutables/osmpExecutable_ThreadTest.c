/******************************************************************************
* FILE: osmpExecutable_ThreadTest.c
* DESCRIPTION:
* OSMP program that calls OSMP_Send/OSMP_Recv to test communication between a pair of threads.
*
* AUTOR : Sanae Sandi & Ilham Asnina, Jun 18, 2024
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../osmpLibrary/OSMP.h"

#define THREAD_NUMS 20

void* sendRoutine(void* args);
void* recvRoutine(void* args);

int main(int argc, char* argv[]) {
    int size, rank;

    OSMP_Init(&argc, &argv);
    OSMP_Size(&size);
    OSMP_Rank(&rank);
    if(size != 2) {
        printf("Test has to be done with only 2 processes\n");
        return OSMP_FAILURE;
    }

    pthread_t threads[THREAD_NUMS];

    for(int i = 0; i < THREAD_NUMS; i++) {
        if(i % 2 == 0) {
            int* dest = malloc(sizeof(int));
            if(rank % 2 == 0) {
                *dest = rank + 1;
            } else {
                *dest = rank - 1;
            }

            if(pthread_create(&threads[i], NULL, sendRoutine, dest) != 0) {
                printf("Error creating thread\n");
                return OSMP_FAILURE;
            }
        } else {
            if(pthread_create(&threads[i], NULL, recvRoutine, NULL) != 0) {
                printf("Error creating thread\n");
                return OSMP_FAILURE;
            }
        }
    }

    for(int i = 0; i < THREAD_NUMS; i++) {
        if(pthread_join(threads[i], NULL) != 0) {
            printf("Error joining thread\n");
            return OSMP_FAILURE;
        }
    }
    OSMP_Finalize();
    return OSMP_SUCCESS;
}

void* sendRoutine(void* args) {
    int dest = *(int*)args;
    char* sendBuffer = "hello world";
    size_t bufferSize = strlen(sendBuffer) + 1;

    printf("Sending to %d: %s\n", dest, sendBuffer);
    if(OSMP_Send(sendBuffer, (int)bufferSize, OSMP_BYTE, dest) == OSMP_FAILURE) {
        printf("Error in OSMP_Send\n");
        exit(1);
    }
    free(args); // Free the dynamically allocated memory
    return NULL;
}

void* recvRoutine(void* args) {
    (void)args; // Avoid unused parameter warning
    int source, len;
    char recvBuffer[20];

    if(OSMP_Recv(recvBuffer, sizeof(recvBuffer), OSMP_BYTE, &source, &len) == OSMP_FAILURE) {
        printf("Error in OSMP_Recv\n");
        exit(1);
    }
    printf("Received from %d: %s\n", source, recvBuffer);
    return NULL;
}
