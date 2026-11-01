/******************************************************************************
* FILE: osmpExecutable_PostBoxLimitTest.c
* DESCRIPTION:
* Program tests the OSMP system's behavior when the message limit for a single process's mailbox (postbox) is exceeded.
* Autor: Soufiane Baeloul, Yahya Foundou
* Configuration: 2 ./osmpExecutable_PostBoxLimitTest
******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include "../osmpLibrary/OSMP.h"

int main(int argc, char *argv[])
{
    int  size, rank, source;
    int bufin[2], bufout[2], len;
     OSMP_Init( &argc, &argv );
     OSMP_Size( &size );
     OSMP_Rank( &rank );
    if( size != 2 ) { printf("Falsche Prozessanzahl\n");
        return OSMP_FAILURE;
    }
    if( rank == 0 )
    { // OSMP process 0
        bufin[0] = 4711;
        bufin[1] = 4712;
        for(int i = 0; i < OSMP_MAX_MESSAGES_PROC + 1; i++) {
             OSMP_Send(bufin, 2, OSMP_INT, 1);
            printf("Senden Nr. %d abgeschlossen\n", i);
        }
    }
    else{// process 1
        sleep(5);
        printf("Recv aufgewacht\n");
        OSMP_Recv( bufout, 2, OSMP_INT, &source, &len );
    }
     OSMP_Finalize();
    return OSMP_SUCCESS;
}
