#ifndef CUSTOM_H
#define CUSTOM_H

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../wrapper/wrapper.h"

void sig_child_handler(int signum);

typedef struct node
{
    int index;
    struct node* next;
}node;

typedef struct
{
    node* head;
}stack;
#endif