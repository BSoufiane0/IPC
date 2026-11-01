#ifndef BETRIEBSSYSTEME_OSMPLIB_H
#define BETRIEBSSYSTEME_OSMPLIB_H

#define _GNU_SOURCE

#include <semaphore.h>
#include "OSMP.h"

// Name of the shared memory
#define shmName "/shm_36"

/**
 * Macro used to convince the compiler that a variable is being used.
 */
#define UNUSED(x) { (void)(x); }



/** Structure of the Barrier */
typedef struct {
    int count;
    int cycle;
    pthread_mutex_t barrier_Mutex;
    pthread_cond_t condition;
} Barrier;

/** Structure of a message slot */
typedef struct {
    int source; // Rank of the source process
    char message[OSMP_MAX_PAYLOAD_LENGTH]; // Message content 1024
    int count; // Count of elements in the message
    OSMP_Datatype datatype; // Data type of the message
} MessageSlot;

typedef struct {
    int freelst[OSMP_MAX_SLOTS];// index list of free messageslots to use (z.B. Send ). Help function to find the indexes of the free remaining message slots
    int count; // point at the first empty slot, initialised by count = OSMP_MAX_SLOTS (herunterzählen)
    pthread_mutex_t mutex_count;// protects the int count so that we can use it ( use the free slot, which the count points at )
    sem_t messageEmpty; // empty slot
} Freelist_t;

/* Structure of a process */
typedef struct {
    int pid; // Process ID
    int rank;
    // kan char rditu int to test
    char PostEingang[OSMP_MAX_MESSAGES_PROC]; // Incoming posts for the process 16, it should be used as a ring buffer. that has two indexes: proc_in and proc_out
    int proc_in; // Index of the incoming message
    pthread_mutex_t mutex_proc_in; // Semaphore for incoming message synchronization
    int proc_out; // Index of the outgoing message
    pthread_mutex_t mutex_proc_out; // Semaphore for outgoing message synchronization
    sem_t sem_proc_full; // Semaphore indicating full incoming message buffer
    sem_t sem_proc_empty; // Semaphore indicating empty incoming message buffer
    MessageSlot gather_slot; // Gather message for the process
    pthread_mutex_t mutex_gather_slot; // mutex synchronizer
} Prozess;

/* Structure of the Shared Memory */
typedef struct {
    int anzahlProz; // Number of processes
    size_t shmlen; // length of the shared memory; can be used in other functions to verify that the shm exists
    pthread_mutex_t loggingMutex; //mutex for logger
    pthread_mutex_t mutex_shm;// mutex shm
    Freelist_t freelst;// warteschlange
    Barrier barrier;//
    Barrier barriergather;
    MessageSlot msg[OSMP_MAX_SLOTS]; // Array of message slots 256
    Prozess P[]; // Array of processes
} SharedMemory;

/* Information about non-blocking operations */
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


/*Documentation of the used functions in osmplib.c*/
/**
 * @brief Wartet auf ein Semaphor.
 * Diese Funktion führt einen sem_wait-Aufruf auf dem übergebenen Semaphor durch.
 * @param sem Zeiger auf das Semaphor.
 * @return OSMP_SUCCESS bei Erfolg, OSMP_FAILURE bei Fehler.
 */
int semwait(sem_t *sem);


/**
 * @brief Signalisiert ein Semaphor.
 * Diese Funktion führt einen sem_post-Aufruf auf dem übergebenen Semaphor durch.
 * @param sem Zeiger auf das Semaphor.
 * @return OSMP_SUCCESS bei Erfolg, OSMP_FAILURE bei Fehler.
 */
int semsignal(sem_t *sem);


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
 * @brief Setzt eine Barriere zurück (function helper for osmp_barrier).
 * Diese Funktion setzt die Barriere zurück, indem der Zähler auf die Anzahl der Prozesse gesetzt
 * und der Zyklus umgeschaltet wird.
 * @param barrier Zeiger auf die Barriere-Struktur.
 */
void barrier_reset(Barrier *barrier);


/**
 * @brief Implementiert eine Barriere-Synchronisation.
 * Diese Funktion implementiert eine Barriere-Synchronisation, bei der alle beteiligten Prozesse
 * warten, bis alle den Synchronisationspunkt erreicht haben.
 * @param barrier Zeiger auf die Barriere-Struktur.
 * @return OSMP_SUCCESS bei Erfolg, OSMP_FAILURE bei Fehler.
 */
int barrier(Barrier *barrier);


/**
 * @brief Implementiert eine Gather-Barriere-Synchronisation in der OSMP-Umgebung (second function for barrier used in gather).
 * Diese Funktion ruft die Barriere-Funktion für eine Gather-Synchronisation der Prozesse auf.
 * @return OSMP_SUCCESS bei Erfolg, OSMP_FAILURE bei Fehler.
 */
int OSMP_BarrierGather(void);



/**
 * @brief Sendet Nachrichten asynchron mithilfe eines Threads.
 * Diese Funktion wird in einem separaten Thread ausgeführt und sendet Nachrichten blockierend.
 * @param arg Zeiger auf die Request-Struktur.
 * @return NULL
 */
void *pthreadSend(void *arg);


/**
 *  Empfängt Nachrichten mithilfe der Funktion OSMP_Recv und aktualisiert das 'Flag' der Anforderungsstruktur
 *  basierend auf dem Erfolg der Empfangsoperation.
 *  Nach der Aktualisierung werden die erforderlichen Semaphoren freigegeben .
 * @param arg
 * @return
 */
void *pthreadRecv(void *arg);


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
void log_message_with_timestamp(const char *format, ...);


/**
 * @brief Fügt eine nicht blockierende Anfrage zur verketteten Liste hinzu.
 * Diese Funktion fügt eine neue Anfrage zur verketteten Liste der Anfragen hinzu.
 * Die Liste wird verwendet, um alle laufenden Anfragen zu verfolgen, damit sie
 * später bei Bedarf bereinigt werden können.
 * @param request Ein Zeiger auf die hinzuzufügende Anfrage.
 */
void addRequestToList(Request *request);

/**
 * @brief Entfernt eine nicht blockierende Anfrage aus der verketteten Liste.
 * Diese Funktion entfernt eine Anfrage aus der verketteten Liste der Anfragen.
 * Die Anfrage wird anhand ihres Zeigers gesucht und aus der Liste entfernt.
 * @param request Ein Zeiger auf die zu entfernende Anfrage.
 */
void removeRequestFromList(Request *request);

/**
 * @brief Bereinigt die verkettete Liste der nicht blockierenden Anfragen.
 * Diese Funktion durchläuft die gesamte verkettete Liste der Anfragen und entfernt
 * jede einzelne. Dabei werden alle Ressourcen, die mit jeder Anfrage verbunden sind,
 * freigegeben, einschließlich Semaphore und Mutexes.
 * Diese Funktion wird in der Regel beim Programmabschluss aufgerufen, um sicherzustellen,
 * dass keine Anfragen mehr laufen und alle Ressourcen ordnungsgemäß freigegeben wurden.
 */
void cleanupRequestList();

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


#endif //BETRIEBSSYSTEME_OSMPLIB_H
