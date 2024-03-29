#include <linux/my_mpi.h>
#include <asm-i386/errno.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

// GLOBALS
list_t g_mpi_head;
BOOL didInit = FALSE; 
int nextRank = 0;

int sys_register_mpi(void)
{
    printk("starting register\n");
    if (!didInit) //init g_mpi_head if it not yet initialized
    { 
        INIT_LIST_HEAD(&g_mpi_head);
        didInit = TRUE;
    }
    if(current->rank != -1) // already registered
        return current->rank;
    // start register process
    INIT_LIST_HEAD(&current->taskMsgHead);
    g_mpi_t* newNode = (g_mpi_t*)kmalloc(sizeof(g_mpi_t),GFP_KERNEL);
    if(newNode == NULL)
        return -ENOMEM;
    // update the node and the task struct to register the process.
    current->rank = nextRank++;
    current->waitingFor = -1;
    newNode->rank = current->rank;
    newNode->taskMsgHead = &current->taskMsgHead;
    newNode->tsk = current;
    list_add_tail( &(newNode->mylist), &g_mpi_head );
    printk("registered new process, rank: %d, mpi list node address: %d", current->rank, newNode);
    return current->rank;
}

int sys_send_mpi_message(int rank, const char* message, ssize_t message_size)
{
    printk("Entered sys_send, sending to rank: %d, from rank %d\n", rank, current->rank);
    // check parameter
    if ( message == NULL || message_size < 1)
        return -EINVAL;
    // make sure current process is registered in the mpi
    if ( current->rank == -1)
    {
        printk("current process not registered, leaving sys_send with error\n");
        return -ESRCH;
    }
    // search for rank in the linked list
    list_t* pos;
    BOOL found = FALSE;
    list_for_each(pos,&g_mpi_head)
    {
        g_mpi_t *t = list_entry(pos,g_mpi_t, mylist);
        if( t->rank == rank)
        {
            found = TRUE;
            break;
        }
    }
    if (found == FALSE)
    {
        printk("Rank to send to is not registered, leaving sys_send with error\n");
        return -ESRCH; // rank was not found in the mpi processes list
    }
    g_mpi_t *recMpiNode = list_entry(pos,g_mpi_t, mylist);
    // both sender and receiver are in the mpi list, copy the message from the user
    char* copiedMsg = (char*)kmalloc(message_size*sizeof(char), GFP_KERNEL);
    if ( copiedMsg == NULL )
        return -ENOMEM;
    if ( copy_from_user(copiedMsg, message, message_size) )
    {
        kfree(copiedMsg);
        return -EFAULT;
    }
    // done copying, create new node for the message list
    msg_q_t* msgNode = (msg_q_t*)kmalloc(sizeof(msg_q_t), GFP_KERNEL);
    if ( msgNode == NULL )
    {
        kfree(copiedMsg);
        return -ENOMEM;
    }
    // update node data, and add to the message list
    msgNode->msg = copiedMsg;
    msgNode->msgsize = message_size;
    msgNode->senderRank = current->rank;
    list_add_tail( &(msgNode->mylist), recMpiNode->taskMsgHead );
    // wakeup receiver
    if( recMpiNode->tsk->waitingFor == current->rank )
    {
        printk("trying to wake up receiver\n");
        wake_up_process(recMpiNode->tsk);
    }
    printk("Done writing message\n");
    return 0;
}

int sys_receive_mpi_message(int rank, int timeout, char* message, ssize_t message_size)
{
    int startT = CURRENT_TIME;
    printk("Entered sys_receive, start time is: %d, rank: %d is trying to receive from rank: %d\n",startT, current->rank, rank);
    // check parameters
    if ( message == NULL || message_size < 1 || timeout < 0)
        return -EINVAL;
    // make sure current process is registered in the mpi
    if ( current->rank == -1)
    {
        printk("current process not registered, leaving sys_receive with error\n");
        return -ESRCH;
    }
    // make sure the receiver rank is valid (smaller than the biggest rank in the system) 
    list_t *pos;
    BOOL found = FALSE;
    BOOL inList = FALSE;

    list_for_each(pos,&g_mpi_head)
    {
        if( list_entry(pos,g_mpi_t, mylist)->rank == rank)
        {
            inList = TRUE;
            break;
        }
    }

    if ( inList == FALSE ) // rank is not in mpi list, exited before we called receive
    {
        printk("Rank to receive from is not registered, leaving sys_receive with error\n");
        return -ESRCH; // rank was not found in the mpi processes list
    }

    do {
        // check if sender is still in mpi list
        inList = FALSE;
        list_for_each(pos,&g_mpi_head)
        {
            if( list_entry(pos,g_mpi_t, mylist)->rank == rank)
            {
                inList = TRUE;
                break;
            }
        }
        // both sender and receiver are in the mpi list,
        // check if there is a message waiting from 'rank'
        list_for_each(pos,&(current->taskMsgHead))
        {
            if( list_entry(pos,msg_q_t, mylist)->senderRank == rank)
            {
                found = TRUE;
                break;
            }
        }
        if ( found == FALSE ) // no message waiting from rank, even though rank is indeed in the mpi list
        {
            int elapsedTime = CURRENT_TIME - startT;
            if(inList == FALSE) { // process to receive from exited without sending message
                printk("Rank to receive from is not registered, leaving sys_receive with error\n");
                return -ESRCH; // rank was not found in the mpi processes list
            }
            else if( elapsedTime < timeout ) // still have timeout, need to sleep
            {
                printk("Rank %d is going to sleep for %d seconds\n", current->rank, timeout - elapsedTime);
                set_current_state(TASK_INTERRUPTIBLE);
                current->waitingFor = rank;
                schedule_timeout((timeout - elapsedTime) * HZ);
                current->waitingFor = -1;
            }
            else { // we passed timeout, and no message, return error
                printk("Timed out while waiting for a message from rank: %d\n", rank);
                return -ETIMEDOUT;
            }
        }
    } while (found == FALSE);
    // found is TRUE, so pos is the relevant message entry from rank to the receiver
    msg_q_t* curMsg = list_entry(pos,msg_q_t, mylist);
    int amntToRead = (curMsg->msgsize < message_size)? curMsg->msgsize : message_size; // copy all if possible, or use up all the room we have
    if ( copy_to_user( message, curMsg->msg , amntToRead) )
    {
        printk("Failed in copy from user\n");
        return -EFAULT;
    }
    ssize_t copiedSize = amntToRead; 
    // done copying the message, remove the entry
    list_del(pos);
    kfree(curMsg->msg);
    kfree(curMsg);
    printk("Done reading message into 'message' buffer, returning copiedSize: %d\n", copiedSize);
    return copiedSize;
}

int copyMPI(struct task_struct* p)
{
    // make sure the parent is actually in the MPI list
    if (current->rank == -1)
        return 0;
    printk("Entered copyMPI from process rank: %d\n", current->rank);
    // init the msg list
    INIT_LIST_HEAD(&p->taskMsgHead);
    // register the child process as a new process in the mpi list
    g_mpi_t* newNode = (g_mpi_t*)kmalloc(sizeof(g_mpi_t),GFP_KERNEL);
    if(newNode == NULL)
        return -ENOMEM; // TODO: what do we do here?
    // update the node and the task struct to register the process.
    p->rank = nextRank++;
    p->waitingFor = -1;
    newNode->rank = p->rank;
    newNode->taskMsgHead = &(p->taskMsgHead);
    newNode->tsk = p;
    list_add_tail( &(newNode->mylist), &g_mpi_head );
    // copy all of the messages from the parent to the child process
    list_t *pos,*n;
    BOOL failed = FALSE;
    list_for_each(pos,&current->taskMsgHead)
    {
        msg_q_t* curMsg = list_entry(pos,msg_q_t, mylist);
        msg_q_t* msgNode = (msg_q_t*)kmalloc(sizeof(msg_q_t), GFP_KERNEL);
        if ( msgNode == NULL)
        {
            failed = TRUE;
            break;
        }
        char* copiedMsg = (char*)kmalloc(curMsg->msgsize * sizeof(char), GFP_KERNEL);
        if ( copiedMsg == NULL)
        {
            failed = TRUE;
            break;
        }
        // copy the message from the parent
        int i;
        for ( i = 0; i <= curMsg->msgsize; ++i)
            copiedMsg[i] = curMsg->msg[i];
        msgNode->msg = copiedMsg;
        msgNode->msgsize = curMsg->msgsize;
        msgNode->senderRank = curMsg->senderRank;
        // finally add the new msg to the list
        list_add_tail( &msgNode->mylist, &p->taskMsgHead);
    }
    // free memory if failed to copy
    if ( failed == TRUE)
    {
        list_for_each_safe(pos,n,&p->taskMsgHead)
        {
            msg_q_t* curMsg = list_entry(pos,msg_q_t, mylist);
            list_del(pos);
            kfree(curMsg->msg);
            kfree(curMsg);
        }
        return -ENOMEM; // TODO: what to do in this situation?
    }
    // done copying all messages
    printk("copyMPI completed, new process rank should be: %d\n", p->rank);
    return 0;
}



void exit_MPI(void)
{
    if(current->rank == -1)
        return; // not registered for MPI
    printk("In exit_MPI, process rank: %d\n", current->rank);
    list_t *pos, *n;
    // we are in MPI, remove us from the mpi process list
    list_for_each(pos,&g_mpi_head)
    {
        if( list_entry(pos,g_mpi_t, mylist)->rank == current->rank)
            break;
    }   
    g_mpi_t *mpiNode = list_entry(pos, g_mpi_t, mylist);
    list_del(pos);
    kfree(mpiNode);
    if (list_empty(&g_mpi_head)) // list is empty, need to reset rank numbers
    {
        nextRank = 0;
        INIT_LIST_HEAD(&g_mpi_head);
    }
    // finally, delete all messages in this process's queue
    list_for_each_safe(pos,n,&current->taskMsgHead)
    {
        msg_q_t* curMsg = list_entry(pos,msg_q_t, mylist);
        list_del(pos);
        kfree(curMsg->msg);
        kfree(curMsg);
    }
    // wakeup any mpi processes that were waiting for us
    list_for_each(pos,&g_mpi_head)
    {
        g_mpi_t *mpiNode = list_entry(pos,g_mpi_t, mylist);
        if( mpiNode->tsk->waitingFor == current->rank )
            set_task_state( mpiNode->tsk, TASK_RUNNING);
    }
    printk("exit_MPI completed, deleted process with rank: %d\n", current->rank);
}
