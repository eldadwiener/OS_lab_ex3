#!/usr/bin/python

from hwgrader.test_utils import tfork2
import MPI
import os
import time


def test1():
    """Basic Test"""
    
    MPI.register_mpi()
    MPI.send_mpi_message(rank=0, message='hello')
    print MPI.receive_mpi_message(rank=0, timeout=1000, message_size=100)

    
def test2():
    """Multi process test"""
    

    MPI.register_mpi()
    
    #
    # Fork the parent
    #
    cpid = os.fork()
    if (cpid == 0):
        #
        # In child
        #
        MPI.send_mpi_message(rank=0, message='hello')
        
        #
        # Terminate the child process
        #
        os._exit(0)

    #
    # Wait for the child to terminate
    #
    os.wait()
    
    #
    # read the message
    #
    print MPI.receive_mpi_message(rank=1, timeout=1000, message_size=100)

    
def test3():
    """Timeout test"""
    
    SLEEP_TIME = 5
    
    MPI.register_mpi()
    
    #
    # Fork the parent
    #
    fork = tfork2()
    if fork.isChild:
        #
        # In child
        #
        print("in child")
        fork.release()
        
        time.sleep(1.1*SLEEP_TIME)
        
        print("child sending hello")
        MPI.send_mpi_message(rank=0, message='hello')
        #
        # Exit the child
        #
        fork.exit()
        
    #
    # In parent
    # ---------
    # Check received message
    #
    print("parent doing sync")
    fork.sync()
    
    t0 = time.time()
    print("parent trying to receive msg")
    print 'recieved message=%s of length=%d' % MPI.receive_mpi_message(rank=1, timeout=2*SLEEP_TIME, message_size=100)
    dt = time.time() - t0
    print 'dt = %g, should be bigger than %g' % (dt, SLEEP_TIME)
    
    #
    # Wait for the child to terminate
    #
    fork.wait()
    

if __name__ == '__main__':
    #test1()
    #test2()
    test3()
    
    
