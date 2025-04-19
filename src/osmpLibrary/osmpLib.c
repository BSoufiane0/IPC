/******************************************************************************
 * FILE: osmpLib.c
 * AUTHOR: Darius Malysiak
 * DESCRIPTION:
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu
 * finden.
 ******************************************************************************/

#include "osmpLib.h"




const char PROCESS_POOL_MEM[] = "/process_pool";
int number_of_sent_message = 0;
ps_pool *pid_pool = NULL;
msg_slots *slots_pool ;
params parameters = {0, 0, NULL} ;
const char SLOTS_MEM[] = "/slots_mem";



int OSMP_GetMaxPayloadLength(void) {
  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_MAX_PAYLOAD_LENGTH;
}

int OSMP_GetMaxSlots(void) {
  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_MAX_SLOTS;
}

int OSMP_GetMaxMessagesProc(void) {
  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_MAX_MESSAGES_PROC;
}

int OSMP_GetFailure(void) {
  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_GetSuccess(void) {
  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_SUCCESS;
}

int OSMP_Init(const int *argc, char ***argv)
{
    /**
     *  the verificatin of argumant parameter
     *  should be done in the parent
     *  or at the beginnof
     *  each process .
     */
    printf("================= start OSMP_Init: %d \n", _getpid());
    parameters = get_parameter(*argc, *argv);

    if (parameters.process_number< 0)
    {
      printf("usage: ./prog -p num");
      exit(EXIT_FAILURE);
    }

    int process_number  ,proc_pool ,slots_pool_fd ;
    size_t size;

    { // init the ps pool ...
      process_number = parameters.process_number;

      proc_pool = _shm_open(PROCESS_POOL_MEM, O_CREAT | O_RDWR, 0666);

      size = sizeof(ps_pool) + (size_t)(process_number * process_number);
      _ftruncate(proc_pool, (off_t) size); // i want to store pid_t

      // printf("map shared memory \n");
      pid_pool = _mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, proc_pool, 0);
      close(proc_pool);
      if (!pid_pool->is_set) // pid_pool first created
      {
        // printf("=======> the pool first initialized ...\n");
        pid_pool->len = 0;
        pid_pool->total_size = process_number + 1; // plus the parent ...
        for (int i = 0; i < pid_pool->total_size; i++)
          pid_pool->pool[i] = NOT_SET;
        pid_pool->is_set = 1;
      }

      // printf("pid_pool meta_data: %d, %d, %d \n", pid_pool->len, pid_pool->is_set, pid_pool->total_size);

      // printf("check overflow ...\n");
      if (pid_pool->len >= pid_pool->total_size){
        printf("overflow ...\n");
        return OSMP_FAILURE; // overflow the buffer size ...
      }
      // add the element and increment len ...
      for (int i = 0; i < pid_pool->total_size; i++)
        if (pid_pool->pool[i] == NOT_SET)
        {
          pid_pool->pool[i] = _getpid();
          pid_pool->len++;
          break;
        }

      
      // else printf("%d - %d - %d\n", pid_pool->len, pid_pool->is_set, pid_pool->total_size);
    }
    { // init msg slots pool ...
      slots_pool_fd = _shm_open(SLOTS_MEM, O_CREAT | O_RDWR, 0666);
      size = sizeof(msg_slots)  + sizeof(message) * (size_t)OSMP_GetMaxSlots();
      _ftruncate(slots_pool_fd, (off_t)size);
      slots_pool = (msg_slots*) _mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, slots_pool_fd, 0);
      close(slots_pool_fd);
      if (slots_pool->total_size != OSMP_GetMaxSlots())
      {
        slots_pool->len = 0;
        slots_pool->total_size = OSMP_GetMaxSlots();
      }
    }
  printf("================= end OSMP_Init: %d \n", _getpid());


    return OSMP_SUCCESS;
}

int OSMP_SizeOf(OSMP_Datatype datatype, unsigned int *size) {
  UNUSED(datatype);
  UNUSED(size);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_Size(int *size)
{
  printf("================= start OSMP_Size: %d \n", _getpid());

    if (pid_pool == NULL)
    {
  #ifdef LOG_DEFINED
      OSMP_Log(OSMP_LOG_FAILS, "OSMP_Size: pid_pool is NULL");
  #endif
      return OSMP_FAILURE;

    }
    *size = pid_pool->len;
  printf("================= start OSMP_Size: %d \n", _getpid());

    return OSMP_SUCCESS;
  }

  int OSMP_Rank(int *rank) {
    printf("================= start OSMP_Rank: %d \n", _getpid());

    if (pid_pool == NULL)
    {
  #ifdef LOG_DEFINED
      OSMP_Log(OSMP_LOG_NONE, "OSMP_Rank: pid_pool is NULL");
  #endif
      return OSMP_FAILURE;
    }
    // printf("start OSMP_Rank \n");
    // printf("call getpid \n");
    pid_t current_pid = _getpid();

    // printf("init rank ptr \n");
    *rank = -1;
    // printf("search for the current pid \n");


    for (int i = 0; i < pid_pool->total_size; i++)
    {
      // printf("search for the current pid %d - pool[%d] = %d \n", current_pid, i, pid_pool->pool[i]);
      if (pid_pool->pool[i] == current_pid)
      {
        *rank = i;
        break;
      }
    }

    if (*rank == -1)
    {
      printf("pid %d not found in pool ...\n", _getpid());
  #ifdef LOG_DEFINED
      OSMP_Log(OSMP_LOG_NONE, "OSMP_Rank: rank not found ...");
  #endif
      printf("=================error end OSMP_Rank: %d \n", _getpid());

      return OSMP_FAILURE;
    }
    printf("================= end OSMP_Rank: %d \n", _getpid());
    return OSMP_SUCCESS;
}

int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {
  UNUSED(buf);
  UNUSED(count);
  UNUSED(datatype);
  UNUSED(dest);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source,
              int *len) {
  UNUSED(buf);
  UNUSED(count);
  UNUSED(datatype);
  UNUSED(source);
  UNUSED(len);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_Finalize(void) {
    // if the pids buffer is not empty, remove the caller and return
  printf("================= start OSMP_Finalize: %d \n", _getpid());

    if (pid_pool == NULL)
    {
  #ifdef LOG_DEFINED
      OSMP_Log(OSMP_LOG_FAILS, "OSMP_Finalize: pid_pool is NULL");
  #endif
      return OSMP_FAILURE;
    }

    pid_t current_pid = _getpid();
    int i;
    size_t size;
    for ( i = 0; i < pid_pool->total_size; i++)
    {
      if (pid_pool->pool[i] == current_pid)
      {
        pid_pool->pool[i] = -1;
        pid_pool->len--;
        break;
      }
    }

    if (i >= pid_pool->total_size)
    {
  #ifdef LOG_DEFINED
      OSMP_Log(OSMP_LOG_FAILS, "OSMP_Finalize: process not found in pool ...");
  #endif
      return OSMP_FAILURE;
    }

    if (pid_pool->len <= 0)
    {
      printf("no process still use the resource ...\n");
      size = sizeof(ps_pool) + (size_t)(pid_pool->total_size * pid_pool->total_size);
      _munmap(pid_pool, size);
      _shm_unlink(PROCESS_POOL_MEM);
      size = sizeof(msg_slots)  + sizeof(message) * (size_t)OSMP_GetMaxSlots();
      _munmap(slots_pool, size);
      _shm_unlink(SLOTS_MEM);
    }
    // otherwise remove the shared memory and associated ressources ...
    printf("================= end OSMP_Finalize: %d \n", _getpid());
    return OSMP_SUCCESS;
}

int OSMP_Barrier(void) {
  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_Gather(void *sendbuf, int sendcount, OSMP_Datatype sendtype,
                void *recvbuf, int recvcount, OSMP_Datatype recvtype,
                int root) {
  UNUSED(sendbuf);
  UNUSED(sendcount);
  UNUSED(sendtype);
  UNUSED(recvbuf);
  UNUSED(recvcount);
  UNUSED(recvtype);
  UNUSED(root);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_ISend(const void *buf, int count, OSMP_Datatype datatype, int dest,
               OSMP_Request request) {
  UNUSED(buf);
  UNUSED(count);
  UNUSED(datatype);
  UNUSED(dest);
  UNUSED(request);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_IRecv(void *buf, int count, OSMP_Datatype datatype, int *source,
               int *len, OSMP_Request request) {
  UNUSED(buf);
  UNUSED(count);
  UNUSED(datatype);
  UNUSED(source);
  UNUSED(len);
  UNUSED(request);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_Test(OSMP_Request request, int *flag) {
  UNUSED(request);
  UNUSED(flag);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_Wait(OSMP_Request request) {
  UNUSED(request);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_CreateRequest(OSMP_Request *request) {
  UNUSED(request);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_RemoveRequest(OSMP_Request *request) {
  UNUSED(request);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}


int OSMP_Log(OSMP_Verbosity verbosity, char *message) {

  if (verbosity < parameters.verbosity) // ignore the msg ...
    return OSMP_SUCCESS;

  if (parameters.log_file == NULL)
  {
    // no log file ...
    parameters.log_file = (char*)_malloc(sizeof(char) * DEFAULT_LOG_FILENAME_SIZE);
    memcpy(parameters.log_file, DEFAULT_LOG_FILE, sizeof(char) * DEFAULT_LOG_FILENAME_SIZE);
  }
  char path[256];
  printf("cwd: %s\n", getcwd(path, sizeof(char)* 256));
  int filed = _open(parameters.log_file, O_CREAT | O_WRONLY | O_APPEND, 0660);
  _write(filed, message, strlen(message));

  _fsync(filed);
  _close(filed);
  return OSMP_SUCCESS;
}




params get_parameter(int argc, char** argv)
{
  int i = 1;
  params p = {-1, -1, NULL};
  if (argc > 7)
  {
    printf("usage: prog -p process_number -l log_file -v verbosity\n");
    exit(1);
  }

  for ( ; i < argc; i++)
  {
    if (!strncmp(argv[i], "-l", 2* sizeof(char)))
    {
      p.log_file = (char*) _malloc(strlen(argv[i+1]) + 1);
      memcpy(p.log_file, argv[i+1], strlen(argv[i+1]) + 1);
    }

    if (!strncmp(argv[i], "-v", 2* sizeof(char)))
    {
      switch (atoi(argv[i+1]))
      {
        case 0:
          p.verbosity = OSMP_LOG_NONE;
          break;
        case 1:
          p.verbosity = OSMP_LOG_BIB_CALL;
          break;
        case 2:
          p.verbosity = OSMP_LOG_MEM;
          break;
        case 3:
          p.verbosity = OSMP_LOG_FAILS;
          break;
      }
    }

    if (!strncmp(argv[i], "-p", 2* sizeof(char)))
    {
      p.process_number = atoi(argv[i+1]);
    }
  }
  return p;
}