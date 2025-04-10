/******************************************************************************
 * FILE: echoall.c
 * AUTHOR: Darius Malysiak
 * DESCRIPTION:
 * First OSMP Executable, it simply echoes all arguments passed to it.
 ******************************************************************************/

#include <stdio.h>

#define UNUSED(x) (void)(x);

int main(int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);
  fprintf(stdout, "Geben Sie hier die Argumente aus.\n");

  fprintf(stdout, "Nun ein neuer Abschnitt, der die pid und ppid ausgibt.\n");

  return 0;
}
