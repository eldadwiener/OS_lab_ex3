#ifndef _MPI_MESSAGES_API
#define _MPI_MESSAGES_API

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


typedef struct msgObj
{
    pid_t pid;
    int rank;
    const char* message;
    ssize_t message_size;
}msgObj;

// Description: Send the message, 'message', of size 'message_size', to rank
// Return value: Success - 0, Failure - -1
int send_mpi_message(int rank, const char* message, ssize_t message_size)
{
    int res;
    __asm__ (
        "pushl %%eax;"
        "pushl %%ebx;"
        "pushl %%ecx;"
        "pushl %%edx;"
        "movl $244, %%eax;"
        "movl %1, %%ebx;"
        "movl %2, %%ecx;"
        "movl %3, %%edx;"
        "int $0x80;"
        "movl %%eax,%0;"
        "popl %%edx;"
        "popl %%ecx;"
        "popl %%ebx;"
        "popl %%eax;"
        : "=m" (res)
        : "m"(rank), "m" (message), "m" (message_size)
    );
    if ((unsigned long)res >= (unsigned long)(-125))
    {
        errno = -res;
        res = -1;
    }
    return (int) res;
}

// Description: Register to the MPI communication list.
// Return value: Success - Rank of process, Failure - -1
int register_mpi(void)
{
    int res;
    __asm__ (
        "pushl %%eax;"
        "movl $243, %%eax;"
        "int $0x80;"
        "movl %%eax,%0;"
        "popl %%eax;"
        : "=m" (res)
    );
    if ((unsigned long)res >= (unsigned long)(-125))
    {
        errno = -res;
        res = -1;
    }
    return (int) res;
}


// Description: Try to receive a message from process "rank" 
// Return value: Success - Size of the message, Failure - -1
int receive_mpi_message(int rank, char *message, ssize_t message_size)
{
    int res;
    __asm__ (
        "pushl %%eax;"
        "pushl %%ebx;"
        "pushl %%ecx;"
        "pushl %%edx;"
        "movl $245, %%eax;"
        "movl %1, %%ebx;"
        "movl %2, %%ecx;"
        "movl %3, %%edx;"
        "int $0x80;"
        "movl %%eax,%0;"
        "popl %%edx;"
        "popl %%ecx;"
        "popl %%ebx;"
        "popl %%eax;"
        : "=m" (res)
        :"m"(rank), "m" (message), "m" (message_size)
    );
    // TODO: Handle post return 
    if ((unsigned long)res >= (unsigned long)(-125))
    {
        errno = -res;
        res = -1;
    }
    return (int) res;
}

#endif
