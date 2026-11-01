/******************************************************************************
* FILE: osmpExecutable_WrongTypeTest.c
* DESCRIPTION:
* Program tests the behavior of the OSMP_Recv function when an incorrect
* datatype is specified. It sends a message with an incorrect datatype from
* process 0 to process 1 and verifies how the OSMP_Recv function behaves when
* attempting to receive this message.
* Autor: Sanae Sandi ,ILham Asnina
* Configuration: 2 ./osmpExecutable_WrongTypeTest
******************************************************************************/
#include <stdio.h>

#include "../osmpLibrary/OSMP.h"


int main(int argc, char *argv[])
{
    int size, rank, source;
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
        OSMP_Send( bufin, 2, 10, 1 );
    }
    else
    { // OSMP process 1
        OSMP_Recv( bufout, 2, 10, &source, &len );
        printf("OSMP process %d received %d byte from %d [%d:%d] \n",rank, len, source, bufout[0], bufout[1]);
    }

    OSMP_Finalize();
    return OSMP_SUCCESS;
}
