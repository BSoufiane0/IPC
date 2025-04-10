/******************************************************************************
 * FILE: osmpLib.c
 * AUTHOR: Darius Malysiak
 * DESCRIPTION:
 * In dieser Quelltext-Datei sind Implementierungen der OSMP Bibliothek zu
 * finden.
 ******************************************************************************/

#include "osmpLib.h"

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

int OSMP_GetSucess(void) {
  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_SUCCESS;
}

int OSMP_Init(const int *argc, char ***argv) {
  UNUSED(argc);
  UNUSED(argv);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_SizeOf(OSMP_Datatype datatype, unsigned int *size) {
  UNUSED(datatype);
  UNUSED(size);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_Size(int *size) {
  UNUSED(size);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}

int OSMP_Rank(int *rank) {
  UNUSED(rank);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
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
  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
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
  UNUSED(verbosity);
  UNUSED(message);

  // TODO: Implementieren Sie hier die Funktionalität der Funktion.
  return OSMP_FAILURE;
}
