#ifndef BETRIEBSSYSTEME_OSMPLIB_H
#define BETRIEBSSYSTEME_OSMPLIB_H

#define _GNU_SOURCE

#include <semaphore.h>
#include <pthread.h>
#include "OSMP.h"


/**
 * Macro used to convince the compiler that a variable is being used.
 */
#define UNUSED(x) { (void)(x); }

#define SHARED_MEMORY_NAME "/shm_25"
#define DEFAULT_LOG_FILE "/home/kali/Workspace/BS/team_25/src/logs/osmp.log"
#define DEFAULT_LOG_FILENAME_SIZE strlen("/home/kali/Workspace/BS/team_25/src/logs/osmp.log")+1

typedef struct {
    int count;
    int cycle;
    int valid;
    int threshold;
    pthread_mutex_t barrier_Mutex;
    pthread_cond_t condition;
} barrier_t;

/** Structure of a message slot */
typedef struct {
    int source; // Rank of the source process
    char message[OSMP_MAX_PAYLOAD_LENGTH]; // Message content 1024
    int count; // Count of elements in the message
    OSMP_Datatype datatype; // Data type of the message
} MessageSlot;

typedef struct {
    // index list of free messageslots to use (z.B. Send ).
    // Help function to find the indexes of the free remaining message slots
    int freelst[OSMP_MAX_SLOTS];
    int count; // point at the first empty slot, initialised by count = OSMP_MAX_SLOTS (herunterzählen)
    pthread_mutex_t mutex_count; // Mutex for the freelist
    sem_t messageEmpty; // empty slot
} Freelist_t;

/* Structure of a process */
typedef struct {
    pid_t pid; // Process ID
    pid_t rank;
    // Incoming posts for the process 16, it should be used as a ring buffer.
    // that has two indexes: proc_in and proc_out
    int PostEingang[OSMP_MAX_MESSAGES_PROC];
    int proc_in; // Index of the incoming message
    pthread_mutex_t in_mutex;
    int proc_out; // Index of the outgoing message
    pthread_mutex_t out_mutex;
    int buffer_len; // Length of the buffer
    pthread_mutex_t mutex_buffer_len; // Mutex for the buffer length
    sem_t sem_proc_empty;
    sem_t sem_proc_full;
    MessageSlot gather_slot; // Gather message for the process
    pthread_mutex_t mutex_gather_slot; // mutex synchronizer
} Prozess;

/* Structure of the Shared Memory */
typedef struct { // the fields should be initialized at the runner ...
    int anzahlProz; // Number of processes
    size_t shmlen; // length of the shared memory; can be used in other functions to verify that the shm exists
    // contains metadata of the msg pool, what is free slots, how many are busy ...
    Freelist_t freelst;
    int next_ps_entry ; // the next entry in Prozess array
    MessageSlot msg[OSMP_MAX_SLOTS]; // Array of message slots 256
    barrier_t barrier;//
    barrier_t barriergather;
    pthread_mutex_t mutex; // Mutex for the shared memory
    pthread_mutex_t log_mutex;
    Prozess P[]; // Array of processes
} SharedMemory;


typedef struct {
    pthread_t thread;// Thread ID
    OOSTATE status_flag; // Status of the request
    const void *sendBuffer; // Buffer used for both send and receive
    int count;// Number of elements of a data type
    int destRank; // Rank of the receiver of a message
    int* source;// Pointer to the rank of the sender
    int* len; // Pointer to the length of the message
    OSMP_Datatype datatype;// Data type of the message
    pthread_mutex_t request_mutex; // Binary semaphore to handle the status flag
    sem_t quit;// Binary semaphore to end the operation and free the memory
} Request;


/*Dynamic List to mark created requests (OSMP_CreateRequest) and removed requests (OSMP_RemoveRequest)*/
typedef struct {
    Request *request;
    struct RequestNode *next;
} RequestNode;



// this manager pack all neccessary shared Memory meta-data ...
typedef struct
{
    SharedMemory* shm_ptr;
    int current_ps_rank;
    size_t shm_memory_size;
    char* log_file;
    OSMP_Verbosity loggingVerbosity;
    pid_t pid;
    int process_child_number;
}ShmMemoryManager;

extern ShmMemoryManager shm_manager;

/** Structure of the barrier_t */

/* Information about non-blocking operations */


/*Dynamic List to mark created requests (OSMP_CreateRequest) and removed requests (OSMP_RemoveRequest)*/





int getenv_param(const int *argc, char ***argv);
int init_shm();
int delete_shm();
/**
 * @brief Überprüft, ob der Shared-Memory-Pointer initialisiert ist.
 * Diese Funktion prüft, ob der übergebene Shared-Memory-Pointer nicht NULL ist.
 * @param ptr Zeiger auf den Shared-Memory-Bereich.
 * @return OSMP_SUCCESS bei Erfolg, OSMP_FAILURE bei Fehler.
 */
int check_shm_ptr(SharedMemory *ptr);


/**
 * @brief Findet einen freien Slot in der Nachrichtenliste.
 * Diese Funktion sucht einen freien Slot in der Nachrichtenliste und gibt dessen Index zurück.
 * @return Index eines freien Slots oder -1 bei Fehler.
 */
int get_free_slot();

/**
 * @brief Gibt einen freien Slot zurück und signalisiert, dass er verfügbar ist.
 * Diese Funktion fügt einen freien Slot in die Freiliste ein und signalisiert,
 * dass ein Slot verfügbar ist, indem das entsprechende Semaphor freigegeben wird.
 * @param msg_slot Der Index des freizugebenden Slots.
 */
void release_free_slot(int msg_slot);




/**
 * @brief Ruft die aktuelle Zeit und das Datum als formatierten String ab.
 * Diese Funktion holt die aktuelle Systemzeit und das Datum, formatiert sie als String.
 * @return Ein Zeiger auf den formatierten Datum- und Zeit-String.
 */
char* OSMP_GetTimeAndDateAsString();

/**
 * @brief Protokolliert eine formatierte Nachricht mit Zeitstempel.
 * Diese Funktion protokolliert eine formatierte Nachricht in eine angegebene Log-Datei,
 * wobei ein Zeitstempel hinzugefügt wird, der angibt, wann die Nachricht protokolliert wurde.
 * @param format Ein Format-String, der mit printf-ähnlicher Formatierung kompatibel ist.
 * @param ... Zusätzliche Argumente, die formatiert und in die Nachricht eingefügt werden sollen.
 *             Diese sollten den Platzhaltern im Format-String entsprechen.
 */
void log_message_with_timestamp(OSMP_Verbosity ,const char *format, ...);



/**
 * @brief Initialisiert die Synchronisationsobjekte wie Bedingungsvariablen, Mutexe und Semaphoren.
 * Diese Funktion initialisiert die notwendigen Synchronisationsobjekte für die gemeinsame
 * Nutzung im Speicherbereich, einschließlich Bedingungsvariablen, Mutexe und Semaphoren.
 * @param ptr Ein Zeiger auf die Struktur des gemeinsam genutzten Speichers.
 * @param prozessAnzahl Die Anzahl der beteiligten Prozesse oder Threads, für die die Synchronisationsobjekte initialisiert werden.
 * @return OSMP_SUCCESS bei Erfolg, andernfalls OSMP_FAILURE.
 */
int initialize_synchronization_objects(SharedMemory *ptr, int prozessAnzahl);

/**
 * @brief Initialisiert die prozessbezogenen Daten (Semaphoren und Mutexe).
 * Diese Funktion initialisiert spezifische prozessbezogene Daten wie Semaphoren und Mutexe
 * im gemeinsam genutzten Speicherbereich, die für die Synchronisation und den Zugriffsschutz benötigt werden.
 * @param ptr Ein Zeiger auf die Struktur des gemeinsam genutzten Speichers.
 */
void initialize_process_data(SharedMemory *ptr);

/**
 * @brief Bereinigt und zerstört die Semaphoren und Mutexe vor dem Beenden.
 * Diese Funktion reinigt und zerstört alle verwendeten Semaphoren und Mutexe,
 * die im gemeinsam genutzten Speicherbereich initialisiert wurden, bevor das Programm beendet wird.
 * @param ptr Ein Zeiger auf die Struktur des gemeinsam genutzten Speichers.
 * @param prozessAnzahl Die Anzahl der beteiligten Prozesse oder Threads, für die die Ressourcen bereinigt werden sollen.
 * @return OSMP_SUCCESS bei Erfolg, andernfalls OSMP_FAILURE.
 */
int cleanup_resources(SharedMemory *ptr, int prozessAnzahl);

int OSMP_Barrier(void);
int OSMP_BarrierGather(void);
int OSMP_Gather(void *sendbuf, int sendcount, OSMP_Datatype sendtype, void *recvbuf, int recvcount, OSMP_Datatype recvtype, int root) ;

#endif //BETRIEBSSYSTEME_OSMPLIB_H


























































































// /******************************************************************************
//  * FILE: osmpLib.h
//  * AUTHOR: Darius Malysiak
//  * DESCRIPTION:
//  * In dieser Datei sind zusätzliche (eigene) Hilfsfunktionen für die **interne**
//  * Verwendung der OSMP-Bibliothek definiert.
//  ******************************************************************************/
//
// #ifndef __OSMP_LIB_H__
// #define __OSMP_LIB_H__
//
//
// #include "OSMP.h"
//
// /**
//  * @brief Dieses Makro wird verwendet, um den Compiler davon zu überzeugen, dass
//  * eine Variable verwendet wird.
//  *
//  * @param x Die zu verwendende Variable
//  */
// #define UNUSED(x) (void)(x);
// #define NOT_SET -1
//
//
//
// typedef struct ps_pool
// {
//     int is_set; // is already set ...
//     int len; // length how many users are already there ...
//     int total_size ; // the whole size ...
//     int free_stack[];
//     pid_t pool[];
// } ps_pool;
//
//
//
// typedef struct header {
//     int dst;
//     int src;
//     int len;
//     OSMP_Datatype type;
// }header;
//
//
// typedef struct msg_slots
// {
//     int len;
//     // int is_set;
//     int dst;
//     int src;
//     OSMP_Datatype type;
//     char message[OSMP_MAX_PAYLOAD_LENGTH];
// } msg_slots;
//
//
//
//
//
// typedef struct
// {
//     int process_number;
//     OSMP_Verbosity verbosity;
//     int is_set_flag;
//     char* log_file;
// } params;
//
//
// extern ps_pool *pid_pool ;
// extern msg_slots *slots_pool ;
//
// params get_parameter(int argc, char** argv);
//
// #endif // __OSMP_LIB_H__
