/******************************************************************************
 * FILE: osmpLib.h
 * AUTHOR: Darius Malysiak
 * DESCRIPTION:
 * In dieser Datei sind zusätzliche (eigene) Hilfsfunktionen für die **interne**
 * Verwendung der OSMP-Bibliothek definiert.
 ******************************************************************************/

#ifndef __OSMP_LIB_H__
#define __OSMP_LIB_H__


#include "OSMP.h"

/**
 * @brief Dieses Makro wird verwendet, um den Compiler davon zu überzeugen, dass
 * eine Variable verwendet wird.
 *
 * @param x Die zu verwendende Variable
 */
#define UNUSED(x) (void)(x);
#define NOT_SET -1




typedef struct ps_pool
{
    int is_set;
    int len;
    int total_size ;
    int pool[];
} ps_pool;







typedef struct header {
    int dst;
    int src;
    int len;
    OSMP_Datatype type;
}header;

typedef struct message
{
    header header;
    void* data;
} message;

typedef struct msg_slots
{
    int len;
    int total_size;
    message msg[];
} msg_slots;




typedef struct
{
    int process_number;
    OSMP_Verbosity verbosity;
    char* log_file;
} params;


extern ps_pool *pid_pool ;
extern msg_slots *slots_pool ;

params get_parameter(int argc, char** argv);

#endif // __OSMP_LIB_H__
