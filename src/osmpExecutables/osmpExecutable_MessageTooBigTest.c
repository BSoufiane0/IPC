/******************************************************************************
* FILE: osmpExecutable_MessageTooBigTest.c
* DESCRIPTION:
* Program tests the OSMP system's behavior when the message size limit is exceeded and if OSMP_Send correctly handles oversized messages by returning OSMP_FAILURE
* Autor: Soufiane Baeloul, Yahya Foundou
* Configuration: 1 ./osmpExecutable_MessageTooBigTest
******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include "../osmpLibrary/OSMP.h"

#define SIZE 2048 // size exceed max size of message [OSMP_MAX_PAYLOAD_LENGTH] = Message content 1024

int main(int argc, char *argv[])
{
    // Initialisierung der Prozesse
    int returncode = OSMP_Init(&argc, &argv);

    // Größe ermitteln
    int size;
    returncode =OSMP_Size(&size);
    if(size != 1) {printf("Falsche Prozessanzahl\n");
        return OSMP_FAILURE;
    }

    // Rang ermitteln
    int rank;
    returncode = OSMP_Rank(&rank);

    OSMP_Datatype datatype = OSMP_BYTE;
    // Speicher reservieren für die zu sendende Nachricht (zu groß für die OSMP_Send Funktion)
    char *send = calloc(1, SIZE);
    if (send == NULL) {
        printf("calloc Fehler\n");
    }
    // Senden der Nachricht
    returncode = OSMP_Send(send, SIZE, datatype, 0);
    if(returncode == OSMP_FAILURE){
        printf("Test bestanden\n");
    } else{
        printf("Falscher Return: %d", returncode);
    }
    OSMP_Finalize();
    return OSMP_SUCCESS;
}
