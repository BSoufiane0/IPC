#define _GNU_SOURCE

#include "osmpRun.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>



int main(int argc, char **argv) {
    printf("Anzahl von Parameter: %d\n", argc);
    if (argc < 3) {
        printf("Usage: ./osmp_run procAnzahl -L /path_to_logger/log.txt -v Verbosity ./osmpExecutable\n");
        exit(EXIT_FAILURE);
    }

    int prozessAnzahl = atoi(argv[1]);
    if (prozessAnzahl <= 0) {
        printf("Die Anzahl der Prozessaufrufe muss eine positive ganze Zahl sein.\n");
        exit(EXIT_FAILURE);
    }

    char* executable;
    char* pathToLoggingFile;
    char** arguments = NULL;
    int loggingVerbosity;
    int fileLength;
    int argumentCounter = 0;
    bool verbositySet = false;

    // Case: First logging parameter given
    if (argv[2][0] == '-' && argv[2][1] == 'L') {
        verbositySet = true;
        pathToLoggingFile = argv[3];
        fileLength = (int)strlen(pathToLoggingFile);

        if (pathToLoggingFile[fileLength - 4] != '.' ||
            pathToLoggingFile[fileLength - 3] != 't' ||
            pathToLoggingFile[fileLength - 2] != 'x' ||
            pathToLoggingFile[fileLength - 1] != 't') {
            printf("Logging file has to be a .txt file\n");
            return OSMP_FAILURE;
        }

        // Case: All logging parameters given
        if (argv[4][0] == '-' && argv[4][1] == 'v') {
            loggingVerbosity = atoi(argv[5]);
            executable = argv[6];

            if (loggingVerbosity < 1 || loggingVerbosity > 3) {
                printf("Wrong logging verbosity\n");
                return OSMP_FAILURE;
            }

            // if (executable[0] != '.' || executable[1] != '/') {
            //     printf("Executable wrongly formatted\n");
            //     return OSMP_FAILURE;
            // }
            // char current_dir[255];
            // printf("%s\n", getcwd(current_dir, sizeof(current_dir)));
            for (int i = 7; argv[i] != NULL; i++) {
                argumentCounter++;
            }

            arguments = malloc(sizeof(char*) * ((size_t)argumentCounter + 4));
            arguments[0] = strdup("1");
            arguments[1] = strdup(pathToLoggingFile);
            arguments[2] = strdup(argv[5]);

            int i, j;
            for (i = 3, j = 7; i <= argumentCounter; i++, j++) {
                arguments[i] = strdup(argv[j]);
            }
            arguments[i] = NULL;

        } else if (argv[4][0] == '.' && argv[4][1] == '/') {
            loggingVerbosity = 1;
            executable = argv[4];

            if (executable[0] != '.' || executable[1] != '/') {
                printf("Executable wrongly formatted\n");
                return OSMP_FAILURE;
            }

            for (int i = 5; argv[i] != NULL; i++) {
                argumentCounter++;
            }

            arguments = malloc(sizeof(char*) * ((size_t)argumentCounter + 4));
            arguments[0] = strdup("1");
            arguments[1] = strdup(pathToLoggingFile);
            arguments[2] = strdup("1");

            int i, j;
            for (i = 3, j = 5; i <= argumentCounter; i++, j++) {
                arguments[i] = strdup(argv[j]);
            }
            arguments[i] = NULL;

        } else {
            printf("Command line wrongly formatted\n");
            return OSMP_FAILURE;
        }

    }
    else if (argv[2][0] == '.' && argv[2][1] == '/') {
        loggingVerbosity = 0;
        executable = argv[2];
        arguments = argv + 3;

        for (int i = 3; argv[i] != NULL; i++) {
            argumentCounter++;
        }

        arguments = malloc(sizeof(char*) * ((size_t)argumentCounter + 2));
        arguments[0] = strdup("0");

        int i, j;
        for (i = 1, j = 3; i <= argumentCounter; i++, j++) {
            arguments[i] = strdup(argv[j]);
        }
        arguments[i] = NULL;

    } else {
        printf("Command line wrongly formatted\n");
        return OSMP_FAILURE;
    }

    FILE* loggingFile = NULL;
    if (verbositySet) {
        loggingFile = fopen(pathToLoggingFile, "a+");
        if (loggingFile == NULL) {
            printf("Logging file couldn't be opened\n");
            printf("logging file = Errno: %s\n", strerror(errno));
            return OSMP_FAILURE;
        }
    }

    int shm_fd = shm_open(shmName, O_CREAT | O_RDWR, (mode_t)0666);
    if (shm_fd == -1) {
        if (loggingVerbosity > 1) {
            fprintf(loggingFile, "Failure: OSMP-Starter / shm_open()\n");
        }
        printf("shm_open Errno: %s\n", strerror(errno));
        if (loggingFile) {
            fclose(loggingFile);
        }
        return OSMP_FAILURE;
    }

    SharedMemory *ptr;
    size_t shmSize = sizeof(SharedMemory) + sizeof(Prozess) * ((size_t)prozessAnzahl);

    if (ftruncate(shm_fd, (long)shmSize) == -1) {
        if (loggingVerbosity > 1) {
            fprintf(loggingFile, "Failure: OSMP-Starter / ftruncate()\n");
        }
        printf("ftruncate - Errno: %s\n", strerror(errno));
        if (loggingFile) {
            fclose(loggingFile);
        }
        return OSMP_FAILURE;
    }

  //  puts("Shm vor mappen"); // added to debug
    ptr = mmap(NULL, shmSize, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    if (close(shm_fd) == -1) {
        if (loggingVerbosity > 1) {
            fprintf(loggingFile, "Failure: OSMP-Starter / close()\n");
        }
        printf("Errno: %s\n", strerror(errno));
        if (loggingFile) {
            fclose(loggingFile);
        }
        return OSMP_FAILURE;
    }
    if (ptr == MAP_FAILED) {
        if (loggingVerbosity > 1) {
            fprintf(loggingFile, "Failure: OSMP-Starter / mmap()\n");
        }
        printf("Errno: %s\n", strerror(errno));
        if (loggingFile) {
            fclose(loggingFile);
        }
        return OSMP_FAILURE;
    }

   // puts("Shm fertig"); // added to debug
    ptr->anzahlProz = prozessAnzahl;

    if (initialize_synchronization_objects(ptr, prozessAnzahl) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            fprintf(loggingFile, "Failure: OSMP-Starter / initialize_synchronization_objects()\n");
        }
        if (loggingFile) {
            fclose(loggingFile);
        }
        return OSMP_FAILURE;
    }
    initialize_process_data(ptr);

    int createdProcesses = 0; // Counter for successfully created processes

    for (int i = 0; i < prozessAnzahl; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            if (loggingVerbosity > 1) {
                fprintf(loggingFile, "Failure: OSMP-Starter / fork()\n");
            }
            printf("fork error - Errno: %s\n", strerror(errno));
            break;
        }
        if (pid == 0) {
            ptr->P[i].pid = getpid();
            ptr->P[i].rank = i;
            execvp(executable, arguments);
            if (loggingVerbosity > 1) {
                fprintf(loggingFile, "Failure: OSMP-Starter / execv()\n");
            }
            printf("exec error - Errno: %s\n", strerror(errno));
            exit(11);
        }
        createdProcesses++;
    }

    if (createdProcesses != prozessAnzahl) {
        for (int j = 0; j < createdProcesses; j++) {
            kill(ptr->P[j].pid, SIGKILL);
        }
    } else {
        int status;
        for (int j = 0; j < createdProcesses; j++) {
            if (waitpid(ptr->P[j].pid, &status, 0) != -1) {
                if (WIFEXITED(status)) {
                    printf("Prozess %d beendet mit Status: %d\n", ptr->P[j].pid, WEXITSTATUS(status));
                } else {
                    perror("WIFEXITED failed");
                    if (loggingVerbosity > 1) {
                        fprintf(loggingFile, "WIFEXITED failed Error\n");
                    }
                    printf("Errno: %s\n", strerror(errno));
                }
            } else {
                if (loggingVerbosity > 1) {
                    fprintf(loggingFile, "wait_pid Error\n");
                }
                printf("Errno: %s\n", strerror(errno));
            }
        }
    }

    if (cleanup_resources(ptr, prozessAnzahl) == OSMP_FAILURE) {
        if (loggingVerbosity > 1) {
            fprintf(loggingFile, "Failure: OSMP-Starter / cleanup_resources()\n");
        }
        printf("Errno: %s\n", strerror(errno));
        if (loggingFile) {
            fclose(loggingFile);
        }
        for (int i = 0; arguments[i] != NULL; i++) {
            free(arguments[i]);
        }
        free(arguments);
        return OSMP_FAILURE;
    }

    if (munmap(ptr, shmSize) == -1) {
        if (loggingVerbosity > 1) {
            fprintf(loggingFile, "Failure: OSMP-Starter / munmap()\n");
        }
        printf("Errno: %s\n", strerror(errno));
        if (loggingFile) {
            fclose(loggingFile);
        }
        for (int i = 0; arguments[i] != NULL; i++) {
            free(arguments[i]);
        }
        free(arguments);
        return OSMP_FAILURE;
    }

    if (shm_unlink(shmName) == -1) {
        if (loggingVerbosity > 1) {
            fprintf(loggingFile, "Failure: OSMP-Starter / shm_unlink()\n");
        }
        printf("Errno: %s\n", strerror(errno));
        if (loggingFile) {
            fclose(loggingFile);
        }
        for (int i = 0; arguments[i] != NULL; i++) {
            free(arguments[i]);
        }
        free(arguments);
        return OSMP_FAILURE;
    }


    if (arguments) {
        for (int i = 0; arguments[i] != NULL; i++) {
            free(arguments[i]);
        }
        free(arguments);
    }

    if (loggingFile) {
        fclose(loggingFile);
    }

    return OSMP_SUCCESS;
}

int initialize_synchronization_objects(SharedMemory *ptr, int prozessAnzahl) {
    // Initialize synchronization objects in shared memory
    ptr->barrier.count = prozessAnzahl;
    ptr->barrier.cycle = 0;

    ptr->barriergather.count = prozessAnzahl;
    ptr->barriergather.cycle = 0;

    pthread_condattr_t condition_attr;
    if (pthread_condattr_init(&condition_attr) != 0) {
        printf("Condattr_init Error\n");
        return OSMP_FAILURE;
    }
    if (pthread_condattr_setpshared(&condition_attr, PTHREAD_PROCESS_SHARED) != 0) {
        printf("Condattr_init_setpshared Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_cond_init(&ptr->barrier.condition, &condition_attr) != 0){
        printf("Cond_init_barrier.condition Error\n");
        return OSMP_FAILURE;
    } // Initialize condition variable for fct Barrier
    if(pthread_cond_init(&ptr->barriergather.condition, &condition_attr) != 0){
        printf("Cond_init_barriergather.condition Error\n");
        return OSMP_FAILURE;
    }// Initialize condition variable for fct BarrierGather

 //   puts("cond fertig");//added to debbug

    pthread_mutexattr_t mutex_attr;
    if (pthread_mutexattr_init(&mutex_attr) != 0) {
        printf("Mutexattr_init Error\n");
        return OSMP_FAILURE;
    }
    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) != 0) {
        printf("Mutexattr_init_setpshared Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_mutex_init(&ptr->loggingMutex, &mutex_attr) !=0){
        printf("Mutex_init loggingMutex Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_mutex_init(&ptr->mutex_shm, &mutex_attr) !=0){
        printf("Mutex_init mutex_shm Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_mutex_init(&ptr->barrier.barrier_Mutex, &mutex_attr)!=0){
        printf("Mutex_init barrier.barrier_Mutex Error\n");
        return OSMP_FAILURE;
    } // Initialize mutex for fct Barrier
    if(pthread_mutex_init(&ptr->barriergather.barrier_Mutex, &mutex_attr)!=0){
        printf("Mutex_init barriergather.barrier_Mutex Error\n");
        return OSMP_FAILURE;
    }// Initialize mutex for fct BarrierGather
    if(pthread_mutex_init(&ptr->freelst.mutex_count, &mutex_attr)!=0){
        printf("Mutex_init freelst.mutex_count Error\n");
        return OSMP_FAILURE;
    }// Initialize mutex for freelist

    for (int i = 0; i < prozessAnzahl; ++i) {
        // Initialize mutexes and semaphores for each process
        if(pthread_mutex_init(&ptr->P[i].mutex_proc_in, &mutex_attr)!=0){
            printf("Mutex_init P[i].mutex_proc_in Error\n");
            return OSMP_FAILURE;
        }// Initialize mutex for fct proc in
        if(pthread_mutex_init(&ptr->P[i].mutex_proc_out, &mutex_attr)!=0){
            printf("Mutex_init P[i].mutex_proc_out Error\n");
            return OSMP_FAILURE;
        }// Initialize mutex for fct proc out
        if(pthread_mutex_init(&ptr->P[i].mutex_gather_slot, &mutex_attr)!=0){
            printf("Mutex_init P[i].mutex_gather_slot Error\n");
            return OSMP_FAILURE;
        }// Initialize mutex for slot
        if(sem_init(&ptr->P[i].sem_proc_full, 1, 0)!=0){
            printf("sem_init P[i].sem_proc_full Error\n");
            return OSMP_FAILURE;
        } // Initialize semaphore proc_full with initial value 0
        if(sem_init(&ptr->P[i].sem_proc_empty, 1, OSMP_MAX_MESSAGES_PROC)!=0){
            printf("sem_init P[i].sem_proc_empty Error\n");
            return OSMP_FAILURE;
        } // Initialize semaphore proc-empty with initial value OSMP_MAX_MESSAGES_PROC
    }

  //  puts("proc mutex fertig");//added to debbug

    for (int i = 0; i < OSMP_MAX_SLOTS; i++) {
        // Initialize the free list
        ptr->freelst.freelst[i] = i;
    }

    if(sem_init(&ptr->freelst.messageEmpty, 1, OSMP_MAX_SLOTS)!=0){
        printf("sem_init freelst.messageEmpty Error\n");
        return OSMP_FAILURE;
    } // Initialize semaphore freelist with initial value OSMP_MAX_SLOTS

    if(pthread_condattr_destroy(&condition_attr)!=0){
        printf("destroy condition_attr Error\n");
        return OSMP_FAILURE;
    }

    if(pthread_mutexattr_destroy(&mutex_attr)!=0){
        printf("destroy barrier.barrier_Mutex Error\n");
        return OSMP_FAILURE;
    }

 //   puts("sem fertig");//added to debbug
    return OSMP_SUCCESS;
}

void initialize_process_data(SharedMemory *ptr) {
    // Initialize the count of free slots
    ptr->freelst.count = OSMP_MAX_SLOTS;
}

int cleanup_resources(SharedMemory *ptr, int prozessAnzahl) {
    // Destroy condition variables
    if(pthread_mutex_destroy(&ptr->loggingMutex)!=0){
        printf("destroy logginMutex Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_mutex_destroy(&ptr->mutex_shm)!=0){
        printf("destroy mutex_shm Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_cond_destroy(&ptr->barrier.condition)!=0){
        printf("destroy barrier.condition Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_cond_destroy(&ptr->barriergather.condition)!=0){
        printf("destroy barriergather.condition Error\n");
        return OSMP_FAILURE;
    }

    // Destroy mutexes
    if(pthread_mutex_destroy(&ptr->barrier.barrier_Mutex)!=0){
        printf("destroy barrier.barrier_Mutex Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_mutex_destroy(&ptr->barriergather.barrier_Mutex)!=0){
        printf("destroy barriergather.barrier_Mutex Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_mutex_destroy(&ptr->freelst.mutex_count)!=0){
        printf("destroy freelst.mutex_count Error\n");
        return OSMP_FAILURE;
    }

    if(sem_destroy(&ptr->freelst.messageEmpty)!=0){
        printf("destroy freelst.messageEmpty Error\n");
        return OSMP_FAILURE;
    } // Destroy semaphore

    for (int i = 0; i < prozessAnzahl; i++) {
        // Destroy mutexes and semaphores for each process
        if(pthread_mutex_destroy(&ptr->P[i].mutex_proc_in)!=0){
            printf("destroy P[i].mutex_proc_in Error\n");
            return OSMP_FAILURE;
        }
        if(pthread_mutex_destroy(&ptr->P[i].mutex_proc_out)!=0){
            printf("destroy P[i].mutex_proc_out Error\n");
            return OSMP_FAILURE;
        }
        if(pthread_mutex_destroy(&ptr->P[i].mutex_gather_slot)!=0){
            printf("destroy P[i].mutex_gather_slot Error\n");
            return OSMP_FAILURE;
        }
        if(sem_destroy(&ptr->P[i].sem_proc_full)!=0){
            printf("destroy P[i].sem_proc_full Error\n");
            return OSMP_FAILURE;
        }
        if(sem_destroy(&ptr->P[i].sem_proc_empty)!=0){
            printf("destroy P[i].sem_proc_empty Error\n");
            return OSMP_FAILURE;
        }
    }
  //  puts("destroy finished");
    return OSMP_SUCCESS;
}
