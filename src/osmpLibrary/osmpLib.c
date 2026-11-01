/**
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu finden.
 */

#define _GNU_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "osmpLib.h"

extern pid_t gettid(void);

SharedMemory *shm_ptr=NULL;
size_t memory_size;
int rankglobal= -1;
pid_t pid;
RequestNode *requestListHead = NULL;
pthread_mutex_t requestListMutex = PTHREAD_MUTEX_INITIALIZER;
//logger params
int loggingVerbosity;
FILE* pathToLoggingFile;


int check_shm_ptr(SharedMemory *ptr) {
    if (ptr == NULL) {
        perror("Error: shared memory nicht initialisiert");
        return OSMP_FAILURE;
    }
    return OSMP_SUCCESS;
}

// Test sem_wait and sem_post
int semwait(sem_t *sem) {
    int semValue = 0;
    sem_getvalue(sem, &semValue);
    if (sem_wait(sem) == -1) {
        if (loggingVerbosity > 1) {
            pthread_mutex_lock(&shm_ptr->loggingMutex);
            fprintf(pathToLoggingFile, "Semwait failed @ PID %d || %s\n", pid, OSMP_GetTimeAndDateAsString());
            pthread_mutex_unlock(&shm_ptr->loggingMutex);
        }
        return OSMP_FAILURE;
    }
    sem_getvalue(sem, &semValue);
    return OSMP_SUCCESS;
}
int semsignal(sem_t *sem){
    int semValue = 0;
    sem_getvalue(sem, &semValue);
    if (sem_post(sem) == -1) {
        if (loggingVerbosity > 1) {
            pthread_mutex_lock(&shm_ptr->loggingMutex);
            fprintf(pathToLoggingFile, "Semsignal failed @ PID %d || %s\n", pid, OSMP_GetTimeAndDateAsString());
            pthread_mutex_unlock(&shm_ptr->loggingMutex);
        }
        return OSMP_FAILURE;
    }
    sem_getvalue(sem, &semValue);
    return OSMP_SUCCESS;
}
void addRequestToList(Request *request) {
    pthread_mutex_lock(&requestListMutex);
    RequestNode *newNode = (RequestNode *) malloc(sizeof(RequestNode));
    if (newNode == NULL) {
        perror("malloc failed in addRequestToList");
        pthread_mutex_unlock(&requestListMutex);
        return;
    }
    newNode->request = request;
    newNode->next = (struct RequestNode *) requestListHead;
    requestListHead = newNode;
    pthread_mutex_unlock(&requestListMutex);
}

void removeRequestFromList(Request *request) {
    pthread_mutex_lock(&requestListMutex);
    RequestNode *current = requestListHead;
    RequestNode *preced = NULL;
    while (current != NULL) {
        if (current->request == request) {
            if(preced == NULL)
                requestListHead = (RequestNode *) current->next;
            else
                preced->next = current->next;
            free(current);
            break;
        }
        current = (RequestNode *) current->next;
    }
    pthread_mutex_unlock(&requestListMutex);
}

void cleanupRequestList() {
    pthread_mutex_lock(&requestListMutex);
    RequestNode *current = requestListHead;
    while (current != NULL) {
        RequestNode *toDelete = current;
        current = (RequestNode *) current->next;
        if(toDelete->request->status_flag == OSMP_RUNNING)
        {
            pthread_cancel(toDelete->request->thread);
        }
        // Destroy the semaphore and mutex associated with the request
        if (sem_destroy(&toDelete->request->quit) != 0) {
            perror("Failure in sem_destroy: requestStruct->quit");
        }
        if (pthread_mutex_destroy(&toDelete->request->request_mutex) != 0) {
            perror("Failure in pthread_mutex_destroy: requestStruct->request_mutex");
        }
        // Free the request structure
        free(toDelete->request);
        // Free the list node
        free(toDelete);
    }
    requestListHead = NULL;
    pthread_mutex_unlock(&requestListMutex);
}

int OSMP_GetMaxPayloadLength() {
    //printf("get_OSMP_MAX_PAYLOAD_LENGTH() gibt den Wert für OSMP_MAX_PAYLOAD_LENGTH zurück\n");
    return OSMP_MAX_PAYLOAD_LENGTH;
}

int OSMP_GetMaxSlots() {
    //printf("get_OSMP_MAX_SLOTS() gibt den Wert für OSMP_MAX_SLOTS zurück\n");
    return OSMP_MAX_SLOTS;
}

int OSMP_GetMaxMessagesProc() {
    //printf("get_OSMP_MAX_MESSAGES_PROC() gibt den Wert für OSMP_MAX_MESSAGES_PROC zurück\n");
    return OSMP_MAX_MESSAGES_PROC;
}

int OSMP_GetFailure() {
    //printf("get_OSMP_FAILURE() gibt den Wert für OSMP_FAILURE zurück\n");
    return OSMP_FAILURE;
}

int OSMP_GetSuccess() {
    //printf("get_OSMP_SUCCESS() gibt den Wert für OSMP_SUCCESS zurück\n");
    return OSMP_SUCCESS;
}

/**
 * @brief Initialize the OSMP environment.
 * This function initializes the OSMP environment by creating a shared memory segment and mapping it into the calling process's address space.
 * @param argc Unused parameter representing the number of command-line arguments.
 * @param argv Unused parameter representing the array of command-line arguments.
 * @return Upon successful initialization, the function returns OSMP_SUCCESS. On failure, it returns OSMP_FAILURE and sets errno appropriately.
 */
int OSMP_Init(const int *argc, char ***argv) {
    UNUSED(argc);
    UNUSED(argv);
    pid = getpid();
    pid_t tid = gettid();

    if (pid != tid) {
        perror("Failed new thread");
        return OSMP_FAILURE;
    }
    int shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Failed to shm_open\n");
        return OSMP_FAILURE;
    }

    struct stat statShm;
    if (fstat(shm_fd, &statShm) == -1) {
        perror("fstat() Fehler");
        return OSMP_FAILURE;
    }
    memory_size = (size_t)statShm.st_size;

    shm_ptr = mmap(NULL, memory_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Failed to mmap");
        return OSMP_FAILURE;
    }

    rankglobal = -1;
    for (int i = 0; i < shm_ptr->anzahlProz; ++i) {
        if (shm_ptr->P[i].pid == pid) {
            rankglobal = shm_ptr->P[i].rank;
            break;
        }
    }

    if (rankglobal == -1) {
        fprintf(stderr, "Process with PID %d could not find its rank.\n", (int)pid);
        return OSMP_FAILURE;
    }

    // Logger part
    int loggingSet = atoi((*argv)[0]);// Convert logging set argument to integer
    if (loggingSet == 0) {// Check if logging set to 0
        loggingVerbosity = 0;  // Set logging verbosity to 0
    } else if (loggingSet == 1) {// if logging set to  1
        char *loggingFileName = (*argv)[1];// Get logging file name from arguments
        pathToLoggingFile = fopen(loggingFileName, "a+");// Open logging file in append mode
        loggingVerbosity = atoi((*argv)[2]);

        if (pathToLoggingFile == NULL) { // Check if logging file couldn't be opened
            printf("Logging file couldn't be opened\n");
            printf("Errno: %s\n", strerror(errno));
            return OSMP_FAILURE;
        }
    } else {
        printf("Wrong logging activity set @ PID %d\n", pid);
        return OSMP_FAILURE;
    }

    log_message_with_timestamp("called OSMP_Init()");// Log message indicating initialization

    return OSMP_SUCCESS;
}

int OSMP_SizeOf(OSMP_Datatype datatype, unsigned int *size) {
    switch (datatype) {
        case OSMP_SHORT:
            *size = sizeof(short);
            break;
        case OSMP_INT:
            *size = sizeof(int);
            break;
        case OSMP_LONG:
            *size = sizeof(long);
            break;
        case OSMP_UNSIGNED_CHAR:
            *size = sizeof(unsigned char);
            break;
        case OSMP_UNSIGNED:
            *size = sizeof(unsigned);
            break;
        case OSMP_UNSIGNED_SHORT:
            *size = sizeof(unsigned short);
            break;
        case OSMP_UNSIGNED_LONG:
            *size = sizeof(unsigned long);
            break;
        case OSMP_FLOAT:
            *size = sizeof(float);
            break;
        case OSMP_DOUBLE:
            *size = sizeof(double);
            break;
        case OSMP_BYTE:
            *size = sizeof(char);
            break;
        default:
            // Handle unsupported datatype
            printf("Unsupported datatype\n");
            return OSMP_FAILURE;
    }

    return OSMP_SUCCESS;
}

/*
 * Funktion liefert die Anzahl der Prozesse in size zurück, die im Shared Memory Bereich registriert sind.
 * Initialisierung der Anzahl findet im OSMP-Starter statt.
 */
int OSMP_Size(int *size) {

    log_message_with_timestamp("called OSMP_Size()");

    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    if (size == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("size is NULL @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    *size = shm_ptr->anzahlProz;
    return OSMP_SUCCESS;
}


/*
 * Funktion liefert den Rang des Prozesses in rank zurück, der ihm im Shared Memory Bereich zugewiesen wurde.
 * Initialisierung des Ranges findet im OSMP-Starter statt.
 */
int OSMP_Rank(int *rank) {
    log_message_with_timestamp("called OSMP_Rank()");

    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    if (rank == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("rank is NULL @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    // Verify the current process's rank
    // if (shm_ptr->P[rankglobal].pid != gettid())
    if (shm_ptr->P[rankglobal].pid != getpid()) {
        return OSMP_FAILURE;
    }

    *rank = rankglobal;
    return OSMP_SUCCESS;
}

// Helper function to find a free message slot für osmpSend
int get_free_slot() {
    int free_slot = -1;
// Wait for a free message slot
    semwait(&(shm_ptr->freelst.messageEmpty));

// Lock the mutex to protect the freelst count
    pthread_mutex_lock(&(shm_ptr->freelst.mutex_count));
    if (shm_ptr->freelst.count > 0) {
        free_slot = shm_ptr->freelst.freelst[--(shm_ptr->freelst.count)];
    }
    pthread_mutex_unlock(&(shm_ptr->freelst.mutex_count));

    return free_slot;
}

int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {
    log_message_with_timestamp("called OSMP_Send()");

    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    if (buf == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Buffer is empty @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    unsigned int dataSize;
    OSMP_SizeOf(datatype, &dataSize);
    if (count <= 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Message is 0 @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    int rank;
    if (OSMP_Rank(&rank) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Error getting rank @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    if (dest < 0 || dest >= shm_ptr->anzahlProz) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Trying to send to a process that's not available @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    } else if (dest == rank) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Can't send to yourself @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    semwait(&(shm_ptr->P[dest].sem_proc_empty));

    int free_slot = get_free_slot();
    if (free_slot == -1) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No free slot available @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    shm_ptr->msg[free_slot].source = rank;
    memcpy(shm_ptr->msg[free_slot].message, buf, (size_t) count * dataSize);
    shm_ptr->msg[free_slot].count = count;
    shm_ptr->msg[free_slot].datatype = datatype;

    pthread_mutex_lock(&(shm_ptr->P[dest].mutex_proc_in));
    shm_ptr->P[dest].PostEingang[shm_ptr->P[dest].proc_in] = (char) free_slot;
    shm_ptr->P[dest].proc_in = (shm_ptr->P[dest].proc_in + 1) % OSMP_MAX_MESSAGES_PROC;
    pthread_mutex_unlock(&(shm_ptr->P[dest].mutex_proc_in));

    semsignal(&(shm_ptr->P[dest].sem_proc_full));

    return OSMP_SUCCESS;
}
//hilfsfunction für osmpRecv
void release_free_slot(int msg_slot) {
    pthread_mutex_lock(&(shm_ptr->freelst.mutex_count));
    shm_ptr->freelst.freelst[shm_ptr->freelst.count++] = msg_slot;
    pthread_mutex_unlock(&(shm_ptr->freelst.mutex_count));
    semsignal(&(shm_ptr->freelst.messageEmpty));
}
int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {
    log_message_with_timestamp("called OSMP_Recv()");

    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    if (buf == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Buffer is empty @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    if (count <= 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Message count is 0 @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    int rank;
    if (OSMP_Rank(&rank) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Error getting rank @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    if (source == NULL || len == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Error source is NULL or len is NULL @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    semwait(&(shm_ptr->P[rank].sem_proc_full));

    pthread_mutex_lock(&(shm_ptr->P[rank].mutex_proc_out));
    int msg_slot = (unsigned char) shm_ptr->P[rank].PostEingang[shm_ptr->P[rank].proc_out];
    shm_ptr->P[rank].proc_out = (shm_ptr->P[rank].proc_out + 1) % OSMP_MAX_MESSAGES_PROC;
    pthread_mutex_unlock(&(shm_ptr->P[rank].mutex_proc_out));

    unsigned int dataSize;
    OSMP_SizeOf(datatype, &dataSize);

    *source = shm_ptr->msg[msg_slot].source;
    *len = shm_ptr->msg[msg_slot].count;

    // Check if the message is received from itself
    if (*source == rank) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Error: Process %d received message from itself @ PID %d ||", rank, pid);
        }
        release_free_slot(msg_slot);
        return OSMP_FAILURE;
    }

    memcpy(buf, shm_ptr->msg[msg_slot].message, (size_t)(*len) * dataSize);

    semsignal(&(shm_ptr->P[rank].sem_proc_empty));

    release_free_slot(msg_slot);

    return OSMP_SUCCESS;
}

// Cleans up resources and resets the state of the OSMP library
int OSMP_Finalize(void) {
    log_message_with_timestamp("called OSMP_Finalize()");

    // Check if shared memory is initialized
    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    // Cleanup remaining requests
    cleanupRequestList();

    // Lock shared memory mutex
    pthread_mutex_lock(&shm_ptr->mutex_shm);

    // Reset process state
    shm_ptr->P[rankglobal].pid = -1;
    shm_ptr->P[rankglobal].proc_in = 0;
    shm_ptr->P[rankglobal].proc_out = 0;
    memset(shm_ptr->P[rankglobal].PostEingang, 0, OSMP_MAX_MESSAGES_PROC); // Reset the array values to 0

    // Unlock shared memory mutex
    pthread_mutex_unlock(&shm_ptr->mutex_shm);

    // Unmap shared memory
    if (munmap(shm_ptr, memory_size) == -1) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Failure: OSMP_Finalize / munmap() @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    // Reset shared memory pointer
    shm_ptr = NULL;

    return OSMP_SUCCESS;
}

// Function helper of barrier
void barrier_reset(Barrier *barrier) {
    barrier->count = shm_ptr->anzahlProz;
    barrier->cycle = !barrier->cycle; //reset for cycle and make sure I'm in another cycle
}

int barrier(Barrier *barrier) {
    log_message_with_timestamp("called barrier()");

    // Check if shared memory is initialized
    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    int status, cancel, tmp, cycle;
    pid_t tid = gettid();

    // Double check if we are at the right process by using the rankglobal
    if (shm_ptr->P[rankglobal].pid != tid) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("We are at the right process comparing pid with tid @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    status = pthread_mutex_lock(&barrier->barrier_Mutex);
    if (status != 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Couldn't lock barrier_mutex @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    cycle = barrier->cycle;
    if (--barrier->count == 0) {
        barrier_reset(barrier);
        if (pthread_cond_broadcast(&barrier->condition) != 0) {
            if (loggingVerbosity > 1) {
                log_message_with_timestamp("Error broadcasting the barrier @ PID %d ||", pid);
            }
        }
    } else {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel);
        while (cycle == barrier->cycle) { // cycles are not changed -> not the event that we want (spurouse wake up)
            status = pthread_cond_wait(&barrier->condition, &barrier->barrier_Mutex);
            if (status != 0) {
                if (loggingVerbosity > 1) {
                    log_message_with_timestamp("Error waiting at the barrier @ PID %d ||", pid);
                }
                break;
            }
        }
        pthread_setcancelstate(cancel, &tmp);
    }

    if (pthread_mutex_unlock(&barrier->barrier_Mutex) != 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Couldn't unlock barrier_mutex @ PID %d ||", pid);
        }
        return OSMP_FAILURE;
    }

    if (status == 0) {
        return OSMP_SUCCESS;
    } else {
        return OSMP_FAILURE;
    }
}
int OSMP_Barrier(void) {
    log_message_with_timestamp("called OSMP_Barrier()");
    return barrier(&shm_ptr->barrier);
}

int OSMP_BarrierGather(void) {
    log_message_with_timestamp("called OSMP_BarrierGather()");
    return barrier(&shm_ptr->barriergather);
}

int OSMP_Gather(void *sendbuf, int sendcount, OSMP_Datatype sendtype, void *recvbuf, int recvcount, OSMP_Datatype recvtype, int root) {
    log_message_with_timestamp("called OSMP_Gather()");

    // Check if shared memory exists
    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if(loggingVerbosity>1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Use the global rank
    int rank = rankglobal;

    // Get the number of processes from shared memory
    int size = shm_ptr->anzahlProz;

    // Check if send buffer is valid
    if (sendbuf == NULL) {
        if(loggingVerbosity>1) {
            log_message_with_timestamp("Send buffer is empty @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if send count is non-positive
    if (sendcount <= 0) {
        if(loggingVerbosity>1) {
            log_message_with_timestamp("Invalid send count @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if receive buffer is valid
    if (recvbuf == NULL) {
        if(loggingVerbosity>1) {
            log_message_with_timestamp("Receive buffer is empty @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if receive count is non-positive
    if (recvcount <= 0) {
        if(loggingVerbosity>1){
            log_message_with_timestamp("Invalid receive count @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if root index is within bounds
    if (root < 0 || root >= size) {
        if(loggingVerbosity>1){
            log_message_with_timestamp("Invalid root process index @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    unsigned int sendSize;
    OSMP_SizeOf(sendtype, &sendSize);

    unsigned int recvSize;
    OSMP_SizeOf(recvtype, &recvSize);

    // Lock the gather mutex of the current process
    if (pthread_mutex_lock(&shm_ptr->P[rank].mutex_gather_slot) != 0) {
        if(loggingVerbosity>1){
            log_message_with_timestamp("Couldn't lock mutex_gather_slot @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Copy the send buffer to the gather slot of the current process
    memcpy(shm_ptr->P[rank].gather_slot.message, sendbuf, (size_t) sendcount * sendSize);
    shm_ptr->P[rank].gather_slot.count = sendcount;
    shm_ptr->P[rank].gather_slot.datatype = sendtype;

    // Unlock the gather mutex
    if (pthread_mutex_unlock(&shm_ptr->P[rank].mutex_gather_slot) != 0) {
        if(loggingVerbosity>1){
            log_message_with_timestamp("Couldn't unlock mutex_gather_slot @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Call the barrier to synchronize all processes
    OSMP_BarrierGather();

    // Root process gathers data from all processes
    if (rank == root) {
        for (int i = 0; i < size; ++i) {
            // Lock the gather mutex of the process
            if (pthread_mutex_lock(&shm_ptr->P[i].mutex_gather_slot) != 0) {
                if(loggingVerbosity>1){
                    log_message_with_timestamp("Couldn't lock mutex_gather_slot for root @ PID %d", pid);
                }
                return OSMP_FAILURE;
            }

            // Copy the data from the gather slot to the root buffer
            memcpy((char *) recvbuf + (size_t) (i * recvcount) * recvSize, // NOTE: THIS SIZE COULD BE WRONG ...
                   shm_ptr->P[i].gather_slot.message,
                   (size_t) shm_ptr->P[i].gather_slot.count * sendSize);

            // Unlock the gather mutex
            if (pthread_mutex_unlock(&shm_ptr->P[i].mutex_gather_slot) != 0) {
                if(loggingVerbosity>1){
                    log_message_with_timestamp("Couldn't unlock mutex_gather_slot for root @ PID %d", pid);
                }
                return OSMP_FAILURE;
            }
        }
    }

    // Call the barrier to synchronize all processes
    OSMP_BarrierGather();

    return OSMP_SUCCESS;
}

void *pthreadSend(void *arg){
    //Um den Thread zu killen
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    Request *requestStruct = arg; // arg in pthread_create uebergeben
    //Thread sendet blockierende Nachricht
    if(OSMP_Send(requestStruct->sendBuffer, requestStruct->count, requestStruct->datatype, requestStruct->destRank)){
        // Falls Nachricht fehlschlaegt
        pthread_mutex_lock(&requestStruct->request_mutex);
        requestStruct->status_flag = OSMP_FAIL;
        pthread_mutex_unlock(&requestStruct->request_mutex);
    } else{
        // Falls erfolgreich Nachricht geschickt
        pthread_mutex_lock(&requestStruct->request_mutex);
        requestStruct->status_flag = OSMP_FINISHED;
        pthread_mutex_unlock(&requestStruct->request_mutex);
    }
    // Finished Semaphore freigeben, damit Wait-Aufruf blockiert, bis Nachricht geschickt wurde
    semsignal(&requestStruct->quit);
    return NULL;
}

void *pthreadRecv(void *arg){
    //Um den Thread zu killen
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    Request *requestStruct = arg; // arg in pthread_create uebergeben
    //Thread empfaengt blockierende Nachricht
    if(OSMP_Recv((void *)requestStruct->sendBuffer, requestStruct->count, requestStruct->datatype, requestStruct->source, requestStruct->len)){
        // Falls Nachricht fehlschlaegt
        pthread_mutex_lock(&requestStruct->request_mutex);
        requestStruct->status_flag = OSMP_FAIL;
        pthread_mutex_unlock(&requestStruct->request_mutex);
    } else{
        // Falls erfolgreich Nachricht geschickt
        pthread_mutex_lock(&requestStruct->request_mutex);
        requestStruct->status_flag = OSMP_FINISHED;
        pthread_mutex_unlock(&requestStruct->request_mutex);
    }
    // Finished Semaphore freigeben, damit Wait-Aufruf blockiert, bis Nachricht geschickt wurde
    semsignal(&requestStruct->quit);
    return NULL;
}

int OSMP_ISend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request) {
    if (loggingVerbosity > 0) {
        log_message_with_timestamp("called OSMP_ISend()");
    }

    // Check if shared memory is initialized
    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if request is provided
    if (request == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("OSMP_ISend request from process is NULL @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Convert request to Request struct
    Request *requestStruct = (Request *) request;

    // Fill requestInfo for Send
    requestStruct->sendBuffer = buf;
    requestStruct->count = count;
    requestStruct->datatype = datatype;
    requestStruct->destRank = dest;

    // Lock request mutex
    if (pthread_mutex_lock(&requestStruct->request_mutex) != 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Couldn't lock request_mutex in OSMP_ISend @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if flag is not OSMP_READY
    if (requestStruct->status_flag != OSMP_READY) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("OSMP_ISend request is not ready yet @ PID %d", pid);
        }
        pthread_mutex_unlock(&requestStruct->request_mutex);
        return OSMP_FAILURE;
    }

    // Set flag to OSMP_RUNNING
    requestStruct->status_flag = OSMP_RUNNING;

    // Unlock request mutex
    if (pthread_mutex_unlock(&requestStruct->request_mutex) != 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("Couldn't unlock request_mutex in OSMP_ISend @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    /* Thread erstellen */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&requestStruct->thread, &attr, pthreadSend, requestStruct)){
        printf("OSMP_ISecv pthread_create Fehler im Prozess!\n");
        return OSMP_FAILURE;
    }
    return OSMP_SUCCESS;
}

int OSMP_IRecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request) {
    if (loggingVerbosity > 0) {
        log_message_with_timestamp("called OSMP_IRecv()");
    }

    // Check if shared memory is initialized
    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if request is provided
    if (request == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("OSMP_Irecv request from process is NULL @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Convert request to Request struct
    Request *requestStruct = (Request *) request;

    // Fill requestInfo for Recv
    requestStruct->sendBuffer = buf;
    requestStruct->count = count;
    requestStruct->datatype = datatype;
    requestStruct->source = source;
    requestStruct->len = len;

    // Lock request mutex
    pthread_mutex_lock(&requestStruct->request_mutex);

    // Check if flag is not OSMP_READY
    if (requestStruct->status_flag != OSMP_READY) {
        pthread_mutex_unlock(&requestStruct->request_mutex);
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("OSMP_Irecv request is not ready yet @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Set flag to OSMP_RUNNING
    requestStruct->status_flag = OSMP_RUNNING;
    pthread_mutex_unlock(&requestStruct->request_mutex);

    /* Thread erstellen (ohne Attribute) */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&requestStruct->thread, &attr, pthreadRecv, requestStruct)){
        printf("OSMP_Irecv pthread_create Fehler im Prozess!\n");
        return OSMP_FAILURE;
    }
    return OSMP_SUCCESS;
}

int OSMP_Test(OSMP_Request request, int *flag) {
    if (loggingVerbosity > 0) {
        log_message_with_timestamp("called OSMP_Test()");
    }
    // Check if shared memory is initialized
    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if request is provided
    if (request == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("OSMP_Test: request from process is NULL @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if flag is provided
    if (flag == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("OSMP_Test: flag is NULL @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Convert request to Request struct
    Request *requestStruct = (Request *) request;

    // Access flag
    pthread_mutex_lock(&requestStruct->request_mutex);
    *flag = (int) requestStruct->status_flag;
    pthread_mutex_unlock(&requestStruct->request_mutex);

    return OSMP_SUCCESS;
}

int OSMP_Wait(OSMP_Request request) {
    if (loggingVerbosity > 0) {
        log_message_with_timestamp("called OSMP_Wait()");
    }

    // Check if shared memory is initialized
    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if request is provided
    if (request == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("OSMP_Wait: request from process is NULL @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Umwandlung auf requestInfo
    Request *requestStruct = (Request *) request;
    //auf beendete Semaphore warten, die nach OSMP_Isend, bzw. OSMP_Irecv freigegeben wird
    semwait(&requestStruct->quit);

    return OSMP_SUCCESS;
}

int OSMP_CreateRequest(OSMP_Request *request) {
    if (loggingVerbosity > 0) {
        log_message_with_timestamp("called OSMP_CreateRequest()");
    }

    // Check if shared memory is initialized
    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Allocate memory for RequestInfo
    Request* requestStruct = (Request*) malloc(sizeof(Request));
    if (requestStruct == NULL) {
        if (loggingVerbosity > 2) {
            log_message_with_timestamp("Couldn't allocate memory for requestStruct @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Initialize status flag and mutex for the request
    requestStruct->status_flag = OSMP_READY;
    pthread_mutexattr_t mutex_attr;
    if (pthread_mutexattr_init(&mutex_attr) != 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("pthread_mutexattr_init error");
        }
        perror("pthread_mutexattr_init");
        free(requestStruct);
        return OSMP_FAILURE;
    }

    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) != 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("pthread_mutexattr_setpshared error");
        }
        perror("pthread_mutexattr_setpshared");
        pthread_mutexattr_destroy(&mutex_attr);
        free(requestStruct);
        return OSMP_FAILURE;
    }

    if (pthread_mutex_init(&requestStruct->request_mutex, &mutex_attr) != 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("pthread_mutex_init request_mutex error");
        }
        perror("pthread_mutex_init");
        pthread_mutexattr_destroy(&mutex_attr);
        free(requestStruct);
        return OSMP_FAILURE;
    }

    if (pthread_mutexattr_destroy(&mutex_attr) != 0) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("pthread_mutexattr_destroy error");
        }
        perror("pthread_mutexattr_destroy");
        pthread_mutex_destroy(&requestStruct->request_mutex);
        free(requestStruct);
        return OSMP_FAILURE;
    }

    // Initialize other fields of the request struct
    requestStruct->thread = 0;
    memcpy(&requestStruct->sendBuffer, "\0", 1);
    requestStruct->destRank = -1;
    requestStruct->count = 0;
    requestStruct->source = NULL;
    requestStruct->len = NULL;
    requestStruct->datatype = 0;

    // semaphor initialisieren
    sem_init(&requestStruct->quit, 1, 0);

    pthread_mutex_lock(&requestStruct->request_mutex);
    requestStruct->status_flag = OSMP_READY;
    pthread_mutex_unlock(&requestStruct->request_mutex);

    addRequestToList(requestStruct);
    // Convert request to OSMP_Request type
    *request = (OSMP_Request) requestStruct;

    return OSMP_SUCCESS;
}

int OSMP_RemoveRequest(OSMP_Request *request) {
    if (loggingVerbosity > 0) {
        log_message_with_timestamp("called OSMP_RemoveRequest()");
    }

    // Check if shared memory is initialized
    if (check_shm_ptr(shm_ptr) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("No shared memory area accessible @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Check if request is NULL
    if (request == NULL || *request == NULL) {
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("RemoveRequest: request is NULL @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }

    // Convert request to Request struct
    Request *requestStruct = (Request *) *request;

    // Check if the thread associated with the request is still running
    pthread_mutex_lock(&requestStruct->request_mutex);
    if (requestStruct->status_flag == OSMP_RUNNING) {
        pthread_mutex_unlock(&requestStruct->request_mutex);
        if (loggingVerbosity > 1) {
            log_message_with_timestamp("OSMP_RemoveRequest: Thread is still running @ PID %d", pid);
        }
        return OSMP_FAILURE;
    }
    pthread_mutex_unlock(&requestStruct->request_mutex);

    if (sem_destroy(&requestStruct->quit) != 0){
        perror("Failure in sem_destroy: requestStruct->quit \n");
        return OSMP_FAILURE;
    }
    // Destroy the mutex associated with the request
    if (pthread_mutex_destroy(&requestStruct->request_mutex) != 0) {
        perror("Failure in pthread_mutex_destroy: requestStruct->request_mutex");
        return OSMP_FAILURE;
    }

    removeRequestFromList(requestStruct);
    // Free the allocated memory for the Request structure
    free(requestStruct);
    if (loggingVerbosity > 2) {
        log_message_with_timestamp("called free on requestStruct");
    }

    *request = NULL;  // Set the request pointer to NULL to avoid dangling pointer issues

    return OSMP_SUCCESS;
}

int OSMP_GetSharedMemoryName(char **name) {
    *name = shmName;

    return OSMP_SUCCESS;
}

char* OSMP_GetTimeAndDateAsString() {
    time_t rawtime;  // Variable to hold raw time
    struct tm *timeinfo;  // Structure to hold time components
    char *buffer = (char*)malloc(30); // Allocate memory for the buffer to hold time string

    if (buffer == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    time(&rawtime);// Get current time in seconds since epoch
    timeinfo = localtime(&rawtime);// Convert raw time to local time representation

    if (strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", timeinfo) == 0) {
        perror("strftime failed");
        free(buffer);
        return NULL;
    }

    return buffer;
}
void log_message_with_timestamp(const char *format, ...) {
    if (loggingVerbosity > 0) {// Check if logging is enabled
        va_list args;  // Variable argument list
        va_start(args, format);  // Initialize variable argument list

        char *timeString = OSMP_GetTimeAndDateAsString();
        if (timeString == NULL) {
            va_end(args);
            return;
        }

        pthread_mutex_lock(&shm_ptr->loggingMutex);

        fprintf(pathToLoggingFile, "PID: %d ", pid);// Print process ID to logging file
        vfprintf(pathToLoggingFile, format, args);  // Print formatted message to logging file
        fprintf(pathToLoggingFile, "\t\t%s\n", timeString);  // Print timestamp to logging file

        pthread_mutex_unlock(&shm_ptr->loggingMutex);

        free(timeString);
        va_end(args);
    }
}
