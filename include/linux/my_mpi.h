#ifndef _MY_MPI_H_
#define _MY_MPI_H_

#include <linux/list.h>
#include <linux/types.h>

typedef struct global_mpi_struct g_mpi_t;
typedef struct msg_queue_struct msg_q_t;


typedef enum BOOL {FALSE,TRUE} BOOL;

struct global_mpi_struct{
    list_t mylist;
    int rank;
    list_t *taskMsgHead;
};

struct msg_queue_struct {
   list_t mylist;
   char* msg;
   int msgsize;
   int senderRank; 
};

int sys_register_mpi(void);
int sys_send_mpi_message(int rank, const char* message, ssize_t message_size);
int sys_receive_mpi_message(int rank, char* message, ssize_t message_size);

int copyMPI(struct task_struct* p);
void exit_MPI(void);

#endif //_MY_MPY_H_
