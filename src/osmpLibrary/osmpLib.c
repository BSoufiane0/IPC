/**
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu finden.
 */

#define _GNU_SOURCE



#include "osmpLib.h"
#include <semaphore.h>
#include <pthread.h>
#include <tgmath.h>
#include <linux/limits.h>


ShmMemoryManager shm_manager;
extern pid_t gettid(void);



int delete_shm()
{
    printf("delete_shm | shm size %ld \n", (long)shm_manager.shm_memory_size);
    if (shm_manager.shm_ptr != NULL && shm_manager.shm_memory_size > 0)
    {
        printf("unmap the shm\n");
        _munmap(shm_manager.shm_ptr, shm_manager.shm_memory_size);
    }
    else
    {
        printf("shm_ptr is null\n");
    }
    printf("unmap the shm\n");
    _shm_unlink(SHARED_MEMORY_NAME);
    printf("delete the shm\n");
    return OSMP_SUCCESS;
}

int init_shm()
{
    shm_manager.shm_memory_size = sizeof(SharedMemory) + sizeof(Prozess) * (size_t)shm_manager.process_child_number;

    int shm_fd = _shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT, 0777);
    ftruncate(shm_fd, (off_t)shm_manager.shm_memory_size);

    shm_manager.shm_ptr = _mmap(0, shm_manager.shm_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    shm_manager.shm_ptr->anzahlProz = shm_manager.process_child_number;
    shm_manager.shm_ptr->shmlen = shm_manager.shm_memory_size;
    shm_manager.shm_ptr->next_ps_entry = 0;
    shm_manager.shm_ptr->freelst.count = OSMP_MAX_SLOTS-1;

    printf("the free list indicator is initialized\n");
    close(shm_fd);
    return OSMP_SUCCESS;
}


int check_shm_ptr(SharedMemory *ptr) {
    if (ptr == NULL) {
        perror("Error: shared memory nicht initialisiert");
        return OSMP_FAILURE;
    }
    return OSMP_SUCCESS;
}


int get_OSMP_MAX_PAYLOAD_LENGTH() {
    //printf("get_OSMP_MAX_PAYLOAD_LENGTH() gibt den Wert für OSMP_MAX_PAYLOAD_LENGTH zurück\n");
    return OSMP_MAX_PAYLOAD_LENGTH;
}

int get_OSMP_MAX_SLOTS() {
    //printf("get_OSMP_MAX_SLOTS() gibt den Wert für OSMP_MAX_SLOTS zurück\n");
    return OSMP_MAX_SLOTS;
}

int get_OSMP_MAX_MESSAGES_PROC() {
    //printf("get_OSMP_MAX_MESSAGES_PROC() gibt den Wert für OSMP_MAX_MESSAGES_PROC zurück\n");
    return OSMP_MAX_MESSAGES_PROC;
}

int get_OSMP_FAILURE() {
    //printf("get_OSMP_FAILURE() gibt den Wert für OSMP_FAILURE zurück\n");
    return OSMP_FAILURE;
}

int get_OSMP_SUCCESS() {
    //printf("get_OSMP_SUCCESS() gibt den Wert für OSMP_SUCCESS zurück\n");
    return OSMP_SUCCESS;
}

/**
 *
 * @param argc parameters length
 * @param argv parameters array
 * @return stores the verbosity and the logging fd at the ShmMemoryManager ...
 */
int getenv_param(const int *argc, char ***argv)
{
    char* logging_file = (char*) malloc(sizeof(char) * PATH_MAX);
    int verbosity = -1;
    int logging = 0;
    for (int i = 1; i < *argc; i++)
    {
        char* tmp = (*argv)[i];
        if (strncmp(tmp, "-v", strlen("-v")) == 0)
        {
            char* cverbosity = (*argv)[i+1];
            verbosity = atoi(cverbosity);
            i++;
        }
        else if (strncmp(tmp, "-l", strlen("-l")) == 0)
        {
            logging = 1;
            strncpy(logging_file, (*argv)[i+1], strlen((*argv)[i+1])+1);
            i++;
        }
        else
            shm_manager.process_child_number = atoi(tmp);

    }

    shm_manager.loggingVerbosity = verbosity == -1 ? OSMP_LOG_NONE : verbosity;

    // open file descriptor or create the file if it does not exists ...
    if (!logging )
        strncpy(logging_file , DEFAULT_LOG_FILE, DEFAULT_LOG_FILENAME_SIZE);

    shm_manager.log_file = logging_file;;
    logging_file = NULL;
    return OSMP_SUCCESS;

}

int get_current_rank()
{
    int current_ps = _getpid();
    for (int i = 0; i < shm_manager.process_child_number; i++)
        if (current_ps == shm_manager.shm_ptr->P[i].pid)
            return i;
    return -1;
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
    /**
     * we expect all direct fields at the SharedMemory object to be initialized at the runner ...
     * TODO: you need to call mutex to synchronize the access to the resource ...
     */

    if (getenv_param(argc, argv) == OSMP_FAILURE)
        return OSMP_FAILURE;

    int memory_fd = _shm_open(SHARED_MEMORY_NAME, O_RDWR, 0777);

    struct stat shm_stat ;
    if (fstat(memory_fd, &shm_stat) < 0)
    {
        OSMP_Log(OSMP_LOG_FAILS, "(OSMP_Init) fstat failed ...\n");
        return OSMP_FAILURE;
    }

    shm_manager.shm_memory_size = (size_t)shm_stat.st_size;

    shm_manager.shm_ptr = (SharedMemory*) _mmap(0, shm_manager.shm_memory_size , PROT_READ | PROT_WRITE, MAP_SHARED, memory_fd, 0);

    if((shm_manager.current_ps_rank = get_current_rank()) < 0)
    {
        OSMP_Log(OSMP_LOG_FAILS, "(OSMP_Init) current_ps_rank is not set ...\n");
        return OSMP_FAILURE;
    }


    {// fill the process structure fields ...
        shm_manager.shm_ptr->P[shm_manager.current_ps_rank].pid = shm_manager.pid = getpid();
        shm_manager.shm_ptr->P[shm_manager.current_ps_rank].proc_in = 0;
        shm_manager.shm_ptr->P[shm_manager.current_ps_rank].proc_out = 0;
        shm_manager.shm_ptr->P[shm_manager.current_ps_rank].rank = shm_manager.current_ps_rank;
    }

    close(memory_fd);

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
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):called OSMP_Size()", get_current_rank());
    if (shm_manager.shm_ptr == NULL)
    {
        OSMP_Log(OSMP_LOG_FAILS, "(OSMP_Size) try to access or trait a non-existed shared Memory ...\n");
        return OSMP_FAILURE;
    }

    *size = shm_manager.shm_ptr->anzahlProz;
    return OSMP_SUCCESS;
}


/*
 * Funktion liefert den Rang des Prozesses in rank zurück, der ihm im Shared Memory Bereich zugewiesen wurde.
 * Initialisierung des Ranges findet im OSMP-Starter statt.
 */
int OSMP_Rank(int *rank) {
    log_message_with_timestamp(OSMP_LOG_INFO, "called OSMP_Rank()");

    if (shm_manager.shm_ptr == NULL)
    {
        OSMP_Log(OSMP_LOG_FAILS, "(OSMP_Rank) try to access or trait a non-existed shared Memory ...\n");
        return OSMP_FAILURE;
    }

    if (shm_manager.current_ps_rank == -1)
    {
        OSMP_Log(OSMP_LOG_FAILS, "(OSMP_Rank) shared Memory initialized, but the rank is not  ...\n");
        return OSMP_FAILURE;
    }

    if (shm_manager.pid != getpid())
    {
        OSMP_Log(OSMP_LOG_FAILS, "(OSMP_Rank) current pid and the shared memeory pid are not identical  ...\n");
        return OSMP_FAILURE;
    }

    *rank = shm_manager.current_ps_rank;
    return OSMP_SUCCESS;
}



int clean_msg_slot(MessageSlot* msg_slot)
{
    msg_slot->source = -1;
    memset(msg_slot->message, 0, sizeof(msg_slot->message));
    msg_slot->count = 0;
    msg_slot->datatype = OSMP_BYTE;
    return OSMP_SUCCESS;
}

int semwait(sem_t *sem) {
    int semValue = 0;
    sem_getvalue(sem, &semValue);
    if (sem_wait(sem) == -1) {
        FILE* file = fopen(shm_manager.log_file, "a");
        pthread_mutex_lock(&shm_manager.shm_ptr->log_mutex);
        fprintf(file, "Semwait failed @ PID %d || %s\n", shm_manager.pid, OSMP_GetTimeAndDateAsString());
        fclose(file);
        pthread_mutex_unlock(&shm_manager.shm_ptr->log_mutex);

        return OSMP_FAILURE;
    }
    sem_getvalue(sem, &semValue);
    return OSMP_SUCCESS;
}

int semsignal(sem_t *sem){
    int semValue = 0;
    sem_getvalue(sem, &semValue);
    if (sem_post(sem) == -1) {
        FILE* file = fopen(shm_manager.log_file, "a");
        pthread_mutex_lock(&shm_manager.shm_ptr->log_mutex);
        fprintf(file, "Semsignal failed @ PID %d || %s\n", shm_manager.pid, OSMP_GetTimeAndDateAsString());
        fclose(file);
        pthread_mutex_unlock(&shm_manager.shm_ptr->log_mutex);
        return OSMP_FAILURE;
    }
    sem_getvalue(sem, &semValue);
    return OSMP_SUCCESS;
}

int get_free_slot()
{
    int free = -1;
    semwait(&(shm_manager.shm_ptr->freelst.messageEmpty));
    pthread_mutex_lock(&(shm_manager.shm_ptr->freelst.mutex_count));
    free = shm_manager.shm_ptr->freelst.freelst[shm_manager.shm_ptr->freelst.count--];
    pthread_mutex_unlock(&(shm_manager.shm_ptr->freelst.mutex_count));
    return free;
}

int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {
    log_message_with_timestamp(OSMP_LOG_INFO, "called OSMP_Send()");

    if (check_shm_ptr(shm_manager.shm_ptr) == OSMP_FAILURE) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "No shared memory area accessible @ PID %d ||", shm_manager.pid);
        return OSMP_FAILURE;
    }

    if (buf == NULL) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Buffer is empty @ PID %d ||", shm_manager.pid);
        return OSMP_FAILURE;
    }

    if (count <= 0) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Message count is 0 @ PID %d ||", shm_manager.pid);
        return OSMP_FAILURE;
    }

    int rank;
    if (OSMP_Rank(&rank) == OSMP_FAILURE) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Error getting rank @ PID %d ||", shm_manager.pid);
        return OSMP_FAILURE;
    }
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message component validation passed", rank);
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message to rank(%d)", rank, dest);
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message waiting for empty pool semaphore", rank);
    semwait(&(shm_manager.shm_ptr->P[dest].is_pool_empty));

    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message empty pool semaphore unlocked", rank);

    pthread_mutex_lock(&(shm_manager.shm_ptr->P[dest].in_mutex));
    pthread_mutex_lock(&(shm_manager.shm_ptr->P[dest].mutex_buffer_len));
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message lock buffer in/out mutexes", rank);

    int in_index = shm_manager.shm_ptr->P[dest].proc_in, buffer_len = shm_manager.shm_ptr->P[dest].buffer_len;

    if ( buffer_len >= OSMP_MAX_MESSAGES_PROC) // the sem is indicated that the pool is empty ..
    {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Error: Process %d cannot send message to process %d @ PID %d ||", rank, dest, shm_manager.pid);
        return OSMP_FAILURE;
    }

    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message pool not full", rank);

    int free_slot = (int)get_free_slot();
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message slot %d is free", rank, free_slot);

    if (free_slot < 0)
    {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Error: Process %d cannot send message to process %d @ PID %d, all slots are busy ||", rank, dest, shm_manager.pid);
        pthread_mutex_unlock(&(shm_manager.shm_ptr->P[dest].in_mutex));
        pthread_mutex_unlock(&(shm_manager.shm_ptr->P[dest].out_mutex));
        return OSMP_FAILURE;
    }

    shm_manager.shm_ptr->msg[(int)free_slot].source = rank;
    shm_manager.shm_ptr->msg[(int)free_slot].count = count;
    shm_manager.shm_ptr->msg[(int)free_slot].datatype = datatype;
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message data is written", rank);
    int copy_len = 0;
    if (OSMP_SizeOf(datatype,(unsigned int*) &copy_len) == OSMP_FAILURE)
    {
        log_message_with_timestamp(OSMP_LOG_FAILS, "<%d> OSMP_Sizeof Error : send message failed ...", rank);
        pthread_mutex_unlock(&(shm_manager.shm_ptr->P[dest].in_mutex));
        pthread_mutex_unlock(&(shm_manager.shm_ptr->P[dest].mutex_buffer_len));
        return OSMP_FAILURE;
    }

    copy_len *= count;

    log_message_with_timestamp(OSMP_LOG_INFO,
        "rank(%d): memcpy src=%p, dest=%p, size=%d",
        rank, buf, shm_manager.shm_ptr->msg[free_slot].message,
        copy_len
    );

    if (memcpy(shm_manager.shm_ptr->msg[free_slot].message, buf, (size_t)copy_len) == NULL)
    {
        log_message_with_timestamp(OSMP_LOG_FAILS, "<%d> Error : %s", rank, strerror(errno));
        pthread_mutex_unlock(&(shm_manager.shm_ptr->P[dest].in_mutex));
        pthread_mutex_unlock(&(shm_manager.shm_ptr->P[dest].mutex_buffer_len));
        return OSMP_FAILURE;
    }
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message data is sent", rank);

    shm_manager.shm_ptr->P[dest].PostEingang[in_index] = free_slot; // set the slot in posteingang
    shm_manager.shm_ptr->P[dest].proc_in = (in_index + 1) % OSMP_MAX_MESSAGES_PROC;
    shm_manager.shm_ptr->P[dest].buffer_len++;
    pthread_mutex_unlock(&(shm_manager.shm_ptr->P[dest].in_mutex));
    pthread_mutex_unlock(&(shm_manager.shm_ptr->P[dest].mutex_buffer_len));


    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message trigger pool full signal", rank);
    semsignal(&(shm_manager.shm_ptr->P[dest].is_pool_full));

    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message ends", rank);

    return OSMP_SUCCESS;
}

void release_free_slot(int msg_slot) {
    pthread_mutex_lock(&(shm_manager.shm_ptr->freelst.mutex_count));
    shm_manager.shm_ptr->freelst.freelst[shm_manager.shm_ptr->freelst.count++] = msg_slot;
    pthread_mutex_unlock(&(shm_manager.shm_ptr->freelst.mutex_count));
    semsignal(&(shm_manager.shm_ptr->freelst.messageEmpty));
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {
    log_message_with_timestamp(OSMP_LOG_INFO, "called OSMP_Recv()");

    if (check_shm_ptr(shm_manager.shm_ptr) == OSMP_FAILURE) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "No shared memory area accessible @ PID %d ||", shm_manager.pid);
        return OSMP_FAILURE;
    }

    if (buf == NULL) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Buffer is empty @ PID %d ||", shm_manager.pid);
        return OSMP_FAILURE;
    }

    if (count <= 0) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Message count is 0 @ PID %d ||", shm_manager.pid);
        return OSMP_FAILURE;
    }

    int rank;
    if (OSMP_Rank(&rank) == OSMP_FAILURE) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Error getting rank @ PID %d ||", shm_manager.pid);
        return OSMP_FAILURE;
    }

    if (source == NULL || len == NULL) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Error source is NULL or len is NULL @ PID %d ||", shm_manager.pid);
        return OSMP_FAILURE;
    }
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):recv message components checked", rank);

    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):recv message waiting for pool full event", rank);

    semwait(&(shm_manager.shm_ptr->P[rank].is_pool_full));
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):recv message pool is full is triggered", rank);

    pthread_mutex_lock(&(shm_manager.shm_ptr->P[rank].out_mutex));
    pthread_mutex_lock(&(shm_manager.shm_ptr->P[rank].mutex_buffer_len));

    int msg_slot = (unsigned char) shm_manager.shm_ptr->P[rank].PostEingang[shm_manager.shm_ptr->P[rank].proc_out];
    shm_manager.shm_ptr->P[rank].proc_out = (shm_manager.shm_ptr->P[rank].proc_out + 1) % OSMP_MAX_MESSAGES_PROC;
    shm_manager.shm_ptr->P[rank].buffer_len--; // decrease the buffer length
    pthread_mutex_unlock(&(shm_manager.shm_ptr->P[rank].out_mutex));
    pthread_mutex_unlock(&(shm_manager.shm_ptr->P[rank].mutex_buffer_len));

    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message slot is checked", rank);

    unsigned int dataSize;
    OSMP_SizeOf(datatype, &dataSize);

    *source = shm_manager.shm_ptr->msg[msg_slot].source;
    *len = shm_manager.shm_ptr->msg[msg_slot].count;
    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):send message data is read", rank);

    // Check if the message is received from itself
    if (*source == rank) {
        log_message_with_timestamp(OSMP_LOG_FAILS, "Error: Process %d received message from itself @ PID %d ||", rank, shm_manager.pid);
        release_free_slot(msg_slot);
        return OSMP_FAILURE;
    }

    memcpy(buf, shm_manager.shm_ptr->msg[msg_slot].message, (size_t)(*len) * dataSize);

    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):recv message trigger is empty event", rank);
    semsignal(&(shm_manager.shm_ptr->P[rank].is_pool_empty));

    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):recv message release busy slot", rank);
    release_free_slot(msg_slot);

    log_message_with_timestamp(OSMP_LOG_INFO, "rank(%d):recv message ends", rank);

    return OSMP_SUCCESS;
}

// Cleans up resources and resets the state of the OSMP library
int OSMP_Finalize(void) {
    log_message_with_timestamp(OSMP_LOG_INFO, "-- OSMP_Finalize() called --");
    if (shm_manager.shm_ptr == NULL)
    {
        OSMP_Log(OSMP_LOG_FAILS, "(OSMP_Finalize) try to access or trait a non-existed shared Memory ...\n");
        return OSMP_FAILURE;
    }

    if (shm_manager.current_ps_rank == -1)
    {
        OSMP_Log(OSMP_LOG_FAILS, "(OSMP_Finalize) shared Memory initialized, but the rank is not set ...\n");
        return OSMP_FAILURE;
    }

    { // clean up the shared entry for the current process ...
        shm_manager.shm_ptr->P[shm_manager.current_ps_rank].pid = -1;
        shm_manager.shm_ptr->P[shm_manager.current_ps_rank].proc_in = 0;
        shm_manager.shm_ptr->P[shm_manager.current_ps_rank].proc_out = 0;
        memset(shm_manager.shm_ptr->P[shm_manager.current_ps_rank].PostEingang, 0, OSMP_MAX_MESSAGES_PROC * sizeof(size_t));
        // clean_msg_slot(&shm_manager.shm_ptr->P[shm_manager.current_ps_rank].gather_slot);
    }
    shm_manager.shm_ptr->next_ps_entry--;

    if (munmap(shm_manager.shm_ptr,shm_manager.shm_memory_size) < 0)
    {
        OSMP_Log(OSMP_LOG_FAILS, "(OSMP_Finalize) munmap failed ...\n");
        return OSMP_FAILURE;
    }
    shm_manager.shm_ptr = NULL;
    free(shm_manager.log_file);
    return OSMP_SUCCESS;
}


int OSMP_ISend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request) {
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(dest);
    UNUSED(request);

    return OSMP_SUCCESS;
}

int OSMP_IRecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request) {
    UNUSED(buf);
    UNUSED(count);
    UNUSED(datatype);
    UNUSED(request);
    UNUSED(source);
    UNUSED(len);

    return OSMP_SUCCESS;
}
// }
//
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
// void log_message_with_timestamp(const char *format, ...) {
//
// }

char* verbosity_to_string(OSMP_Verbosity verbosity)
{
    switch (verbosity)
    {
        case OSMP_LOG_NONE:
            return "OSMP_LOG_NONE";
        case OSMP_LOG_FAILS:
            return "OSMP_LOG_FAILS";
        case OSMP_LOG_MEM:
            return "OSMP_LOG_MEM";
        case OSMP_LOG_BIB_CALL:
            return "OSMP_LOG_BIB_CALL";
    default:
        return "";
    }

}

int OSMP_Log(OSMP_Verbosity verbosity, char *message)
{
    if (verbosity < shm_manager.loggingVerbosity) return OSMP_SUCCESS;

    char* time = OSMP_GetTimeAndDateAsString();
    if (time == NULL) return OSMP_FAILURE;
    char msg[1024];
    snprintf(msg, sizeof(msg), "[%s] -- %s -- %s\n", time, verbosity_to_string(verbosity), message);
    free(time);
    int logfd = _open(shm_manager.log_file, O_WRONLY | O_APPEND | O_CREAT, 0644);

    pthread_mutex_lock(&shm_manager.shm_ptr->log_mutex);
    write(logfd, msg, strlen(msg));
    close(logfd);
    pthread_mutex_unlock(&shm_manager.shm_ptr->log_mutex);

    return OSMP_SUCCESS;
}

void log_message_with_timestamp(OSMP_Verbosity verb, const char *fmt, ...)
{
    va_list va_args;
    va_start(va_args, fmt);
    char buff[4096];
    bzero(buff, sizeof(buff));
    vsnprintf(buff, sizeof(buff), fmt, va_args);
    OSMP_Log(verb, buff);
    va_end(va_args);
}