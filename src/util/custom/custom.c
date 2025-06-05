
#include "custom.h"

#include "../../osmpLibrary/osmpLib.h"

void sig_child_handler(int signum)
{
    UNUSED(signum)
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (WIFEXITED(status))
            printf("Child %d exited with status %d\n", pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child %d killed by signal %d\n", pid, WTERMSIG(status));
    }
}