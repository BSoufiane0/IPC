/******************************************************************************
* FILE: osmpExecutable_InitTest1.c
* DESCRIPTION:
* Program that tests calling other OSMP-Funktionen without initializing OSMP_Init
* Autor: Soufiane Baeloul, Yahya Foundou
* Configuration: 2 ./osmpExecutable_InitTest1
******************************************************************************/


#include <stdio.h>
#include "../osmpLibrary/OSMP.h"
#include "../osmpLibrary/osmpLib.h"


int main (int argc, char** argv){
    UNUSED(argc);
    UNUSED(argv);
    int size, rank, source;
    int bufin[2], bufout[2], len;

    int returncode =OSMP_Size(&size);

    if(returncode == OSMP_FAILURE){
        printf("Test4.1 bestanden\n");
    }


    returncode= OSMP_Rank(&rank);
    if (returncode == OSMP_FAILURE){
        printf("Test4.2 bestanden\n");
    }



    if (OSMP_Send(bufin, 2, OSMP_INT, 1) == OSMP_FAILURE) {
        printf("Test4.3 bestanden\n");
    }

    if (OSMP_Recv(bufout, 2, OSMP_INT, &source, &len) == OSMP_FAILURE) {
        printf("Test4.4 bestanden\n");
    }
    if (OSMP_Finalize() == OSMP_FAILURE) {
        printf("Test4.5 bestanden\n");
    }
    return OSMP_SUCCESS;
}
