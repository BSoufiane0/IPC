#ifndef WRAPPER_H
#define WRAPPER_H
#define _POSIX_C_SOURCE 200809L
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

pid_t _getpid(void);
pid_t _getppid(void);
int _shm_open(const char* name, int oflag, mode_t mode);
int _shm_unlink(const char* name);
void* _mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int _munmap(void* addr, size_t length);
int _ftruncate(int fd, off_t length);
void* _malloc(size_t size);
int _open(const char* path, int oflag, mode_t mode);
ssize_t _write(const int fd, const void* buf, size_t count);
int _fsync(int fd);
int _close(int fd);
#endif