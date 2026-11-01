/******************************************************************************
* FILE: osmpExecutable_GetFunctions.c
* DESCRIPTION:
* Dummy Programm to use the Get-Functions of osmplib
*
* AUTOR : Soufiane Baeloul, Yahya Foundou
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../osmpLibrary/osmpLib.h"



int main(int argc, char *argv[]){
    UNUSED(argc);
    UNUSED(argv);
    int ret;

    ret = OSMP_GetMaxPayloadLength();
    printf("%d\n", ret);

    ret = OSMP_GetMaxSlots();
    printf("%d\n", ret);

    ret = OSMP_GetMaxMessagesProc();
    printf("%d\n", ret);

    ret = OSMP_GetFailure();
    printf("%d\n", ret);

    ret = OSMP_GetSuccess();

    printf("%d\n", ret);


    return 0;
}
