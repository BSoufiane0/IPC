

/******************************************************************************
 * FILE: osmpRun.c
 * AUTHOR: Darius Malysiak
 * DESCRIPTION:
 * Diese Datei enthält die main Funktion für den OSMP-Runner.
 ******************************************************************************/


#include "../osmpLibrary/osmpLib.h"
#include <linux/limits.h>

int init_metadata(SharedMemory* ptr, int prozessAnzahl)
{
    ptr->barrier.count = prozessAnzahl;
    ptr->barrier.cycle = 0;
    ptr->barrier.valid = 1;

    ptr->barriergather.count = prozessAnzahl;
    ptr->barriergather.cycle = 0;
    pthread_condattr_t condition_attr;
    if (pthread_condattr_setpshared(&condition_attr, PTHREAD_PROCESS_SHARED) != 0)
    {
        printf("mutexattr set failed\n");
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

    pthread_mutexattr_t mutexattr;

    if (pthread_mutexattr_init(&mutexattr) != 0)
    {
        printf("mutexattr init failed\n");
        return OSMP_FAILURE;
    }

    if (pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED) != 0)
    {
        printf("mutexattr set failed\n");
        return OSMP_FAILURE;
    }
    if (pthread_mutex_init(&shm_manager.shm_ptr->mutex, &mutexattr) != 0)
    {
        printf("mutex init failed\n");
        return OSMP_FAILURE;
    }

    if (pthread_mutex_init(&shm_manager.shm_ptr->log_mutex, &mutexattr) != 0)
    {
        printf("log mutex init failed\n");
        return OSMP_FAILURE;
    }

    for (int i = 0; i < prozessAnzahl; ++i) {
        log_message_with_timestamp(OSMP_LOG_INFO, "process synchronization stuff of %d initialized", i);
        // Initialize mutexes and semaphores for each process
        if(pthread_mutex_init(&ptr->P[i].in_mutex, &mutexattr)!=0){
            printf("Mutex_init P[i].mutex_proc_in Error\n");
            return OSMP_FAILURE;
        }// Initialize mutex for fct proc in

        if(pthread_mutex_init(&ptr->P[i].out_mutex, &mutexattr)!=0){
            printf("Mutex_init P[i].mutex_proc_out Error\n");
            return OSMP_FAILURE;
        }// Initialize mutex for fct proc out

        if(pthread_mutex_init(&ptr->P[i].mutex_buffer_len, &mutexattr)!=0){
            printf("Mutex_init P[i].mutex_gather_slot Error\n");
            return OSMP_FAILURE;
        }// Initialize mutex for slot

        if(sem_init(&ptr->P[i].sem_proc_empty, 1, OSMP_MAX_MESSAGES_PROC)!=0){
            printf("sem_init P[i].sem_proc_empty Error\n");
            return OSMP_FAILURE;
        } // Initialize semaphore proc-empty with initial value OSMP_MAX_MESSAGES_PROC

        if(sem_init(&ptr->P[i].sem_proc_full, 1, 0)!=0){
            printf("sem_init P[i].sem_proc_full Error\n");
            return OSMP_FAILURE;
        } // Initialize semaphore proc_full with initial value 0

        if(pthread_mutex_init(&ptr->P[i].mutex_gather_slot, &mutexattr)!=0){
            printf("Mutex_init P[i].mutex_gather_slot Error\n");
            return OSMP_FAILURE;
        }// Initialize mutex for slot
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


    if(pthread_mutexattr_destroy(&mutexattr) != 0){
        printf("destroy barrier.barrier_Mutex Error\n");
        return OSMP_FAILURE;
    }
    return OSMP_SUCCESS;
}



int cleanup_resources(SharedMemory *ptr, int prozessAnzahl) {
    // Destroy condition variables
    if(pthread_mutex_destroy(&ptr->log_mutex)!=0){
        printf("destroy logginMutex Error\n");
        return OSMP_FAILURE;
    }
    if(pthread_mutex_destroy(&ptr->mutex)!=0){
        printf("destroy mutex_shm Error\n");
        return OSMP_FAILURE;
    }

    // TODO: destroy barriers ..
    ptr->barrier.valid = 0;
    if(pthread_cond_destroy(&ptr->barrier.condition)!=0){
        printf("destroy barrier.condition Error\n");
        return OSMP_FAILURE;
    }
    ptr->barriergather.valid = 0;
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
    // Destroy free list
    if(pthread_mutex_destroy(&ptr->freelst.mutex_count) != 0){
        printf("destroy freelst.mutex_count Error\n");
        return OSMP_FAILURE;
    }

    if(sem_destroy(&ptr->freelst.messageEmpty)!=0){
        printf("destroy freelst.messageEmpty Error\n");
        return OSMP_FAILURE;
    } // Destroy semaphore

    for (int i = 0; i < prozessAnzahl; i++) {
        // Destroy mutexes and semaphores for each process
        if(pthread_mutex_destroy(&ptr->P[i].in_mutex)!=0){
            printf("destroy P[i].mutex_proc_in Error\n");
            return OSMP_FAILURE;
        }
        if(pthread_mutex_destroy(&ptr->P[i].out_mutex)!=0){
            printf("destroy P[i].mutex_proc_out Error\n");
            return OSMP_FAILURE;
        }
        if(pthread_mutex_destroy(&ptr->P[i].mutex_buffer_len)!=0){
            printf("destroy P[i].mutex_gather_slot Error\n");
            return OSMP_FAILURE;
        }
        if(sem_destroy(&ptr->P[i].sem_proc_empty)!=0){
            printf("destroy P[i].sem_proc_full Error\n");
            return OSMP_FAILURE;
        }
        if(sem_destroy(&ptr->P[i].sem_proc_full)!=0){
            printf("destroy P[i].sem_proc_empty Error\n");
            return OSMP_FAILURE;
        }
    }
  //  puts("destroy finished");
    return OSMP_SUCCESS;
}

char** prepare_argv(int argc, char **argv) {
    char **args = (char**) calloc((size_t)(argc + 1), sizeof(char*));
    for (int i = 0; i < argc; i++)
    {
        args[i] =(char*) calloc(strlen(argv[i]) + 1, sizeof(char));
        snprintf(args[i], strlen(argv[i])+1, "%s", argv[i]);
    }
    args[argc] = NULL;
    return args;
}

int fill_shm(int i)
{
    shm_manager.shm_ptr->P[i].pid = _getpid();
    shm_manager.shm_ptr->next_ps_entry++;
    shm_manager.shm_ptr->P[i].proc_out = 0;
    shm_manager.shm_ptr->P[i].proc_in = 0;
    shm_manager.shm_ptr->P[i].buffer_len = 0;
    shm_manager.shm_ptr->P[i].rank = i;

    log_message_with_timestamp(OSMP_LOG_INFO, "========PID IN fill_shm: %d", shm_manager.shm_ptr->P[i].pid);
    return OSMP_SUCCESS;
}

int main(int argc, char *argv[]) {


    { // get the environment parameters ...
        if (getenv_param(&argc, &argv) != OSMP_SUCCESS)
        {
            printf("extract environment parameters failed\n");
            return OSMP_FAILURE;
        };
        printf( "Environment parameters set\n");
    }



    char **args =  prepare_argv(argc, argv);

    printf("Arguments set \n");

    {
        // initialize the shared memory ...
        // either open it or create it if it does not exist ...
        init_shm();
        log_message_with_timestamp(OSMP_LOG_INFO, "================================\n\n\n");
        log_message_with_timestamp(OSMP_LOG_NONE,"SHM initialized successfully");
    }


    if (init_metadata(shm_manager.shm_ptr, shm_manager.shm_ptr->anzahlProz) == OSMP_FAILURE)
        exit(EXIT_FAILURE);

    log_message_with_timestamp(OSMP_LOG_NONE,"synchronization metadata initialized successfully");

    int forked_ps_indicator = 0;
    { // fork and change the text area of the processes (spawnning)

        for (int i = 0 ; i < shm_manager.process_child_number ; i++)
        {
            pid_t pid = fork();
            if (!pid)
            {
                fill_shm(i);
                printf( "========== PID = %d", shm_manager.shm_ptr->P[i].pid);
                printf( "---------- rank = %d", shm_manager.shm_ptr->P[i].rank);
                // log_message_with_timestamp(OSMP_LOG_NONE,"fork child");

                execv("/home/ms/y/yf151089/CLionProjects/team_25/build/prak4_1", args);

                log_message_with_timestamp(OSMP_LOG_FAILS, "execv failed");
                exit(EXIT_FAILURE); // exit from the child ...
            }
            else if ( pid < 0) // case of error ...
            {
                log_message_with_timestamp(OSMP_LOG_FAILS, "fork failed ...");
                shm_manager.shm_ptr->P[i].pid = (pid_t)-1; // indicate that the rank i contains no ps to kill  ...
                continue;
            }
            forked_ps_indicator++;
        }
        log_message_with_timestamp(OSMP_LOG_NONE,"forking process is done < %d ps is forked successfully>", forked_ps_indicator);
    }


    { // wait for the processes to finish ...
        if (forked_ps_indicator != shm_manager.process_child_number)
            for (int i = 0 ; i < shm_manager.process_child_number ; i++)
                if (shm_manager.shm_ptr->P[i].pid != (pid_t)-1)
                    kill((pid_t)shm_manager.shm_ptr->P[i].pid, SIGKILL);

        for (int i = 0 ; i < shm_manager.process_child_number ; i++)
        {
            waitpid(-1, NULL, 0);
        }
    }
    printf("all processes finished\n");
    log_message_with_timestamp(OSMP_LOG_NONE,"all processes are finished");

    printf("free the tmp array ...\n");
    for (int i = 0 ; i < argc; i++)
    {
        free(args[i]);
        args[i] = NULL;
    }
    free(args);
    args = NULL;
    if (cleanup_resources(shm_manager.shm_ptr, shm_manager.shm_ptr->anzahlProz) == OSMP_FAILURE) {
        FILE* file = fopen(shm_manager.log_file, "a");
        fprintf(file, "Failure: OSMP-Starter / cleanup_resources()\n");

        printf("Errno: %s\n", strerror(errno));
        if (file) {
            fclose(file);
        }

        return OSMP_FAILURE;
    }
    delete_shm();
    return OSMP_SUCCESS;
}



