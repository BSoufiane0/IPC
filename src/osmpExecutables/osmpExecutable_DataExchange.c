/******************************************************************************
* FILE: osmpExecutable_DataExchange.c
* DESCRIPTION:
* Dummy Programm to exchange Data through Shared Memory
*
******************************************************************************/

#include <stdio.h>

#include "../osmpLibrary/OSMP.h"


int main(int argc, char *argv[]){
    int size;
    int rank;
    printf("Hier wurde aus dem Shared Memory gelesen\n");


    OSMP_Init(&argc, &argv);
    OSMP_Size(&size);
    printf("OSMP_Size liefert in Size die Zahl der OSMP-Prozesse ohne OSMP-Starter Prozess zurück %d\n",size);
    OSMP_Rank(&rank);
    printf("OSMP-Rank liefert in *rank die OSMP-Prozessnummer des aufrufenden OSMP-Prozesses zurück: %d\n",rank);


    return 0;
}
