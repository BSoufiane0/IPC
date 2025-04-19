#include "wrapper.h"

#include <errno.h>
#include <string.h>


pid_t _getpid(void)
{
    pid_t pid = getpid();
    if (pid < 0)
    {
        perror("getpid: ");
        exit(EXIT_FAILURE);
    }
    return pid;
}


pid_t _getppid(void)
{
    pid_t pid = getppid();
    if (pid < 0)
    {
        perror("getppid: ");
        exit(EXIT_FAILURE);
    }
    return pid;
}

int _shm_open(const char* name, int oflag, mode_t mode)
{
    int fd = shm_open(name, oflag, mode);
    if (fd < 0)
    {
        perror("shm_open: ");
        exit(EXIT_FAILURE);
    }
    return fd;
}
int _shm_unlink(const char* name)
{
    int ret = shm_unlink(name);
    if (ret < 0)
    {
        perror("shm_unlink: ");
        exit(EXIT_FAILURE);
    }
    return ret;
}
void* _mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    void* ptr = mmap(addr, length, prot, flags, fd, offset);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

int _munmap(void *addr, size_t length)
{
    int rv = munmap(addr, length);
    if (rv < 0)
    {
        perror("munmap: ");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int _ftruncate(int fd, off_t length)
{
    int ret = ftruncate(fd, length);
    if (ret < 0)
    {
        perror("ftruncate: ");
        exit(EXIT_FAILURE);
    }
    return ret;
}


void* _malloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
        perror("malloc: ");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

int _open(const char* path, int oflag, mode_t mode)
{
    int fd = open(path, oflag, mode);
    if (fd < 0)
    {
        printf("open: %s > %s", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

ssize_t _write(const int fd, const void* message, size_t len)
{
    ssize_t nwrite = write(fd, message, len);
    if (nwrite < 0)
    {
        perror("write: ");
        exit(EXIT_FAILURE);
    }
    return nwrite;
}

int _fsync(int fd)
{
    int rv = fsync(fd);
    if (rv < 0)
    {
        perror("open: ");
        exit(EXIT_FAILURE);
    }
    return rv;
}


int _close(int fd)
{
    int rv = close(fd);
    if (rv < 0)
    {
        perror("open: ");
        exit(EXIT_FAILURE);
    }
    return rv;
}












