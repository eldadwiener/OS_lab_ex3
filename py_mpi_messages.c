#include <Python.h>
#include "mpi_messages_api.h"


static PyObject *
posix_error(void)
{
	return PyErr_SetFromErrno(PyExc_OSError);
}


static PyObject *
py_send_mpi_message(PyObject *self, PyObject *args, PyObject *keywds)
{
	int rank;
	char *message;
	int message_size;	
	int status;

	static char *kwlist[] = {"rank", "message", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, keywds, "iz#", kwlist,
					&rank, &message, &message_size))
		return NULL;

	status = send_mpi_message(rank, message, message_size);

	if (status < 0)
		return posix_error();

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *
py_register_mpi(PyObject *self, PyObject *args)
{
	int rank;

	rank = register_mpi();

	if (rank < 0)
		return posix_error();

	return Py_BuildValue("i", rank);
}


static PyObject *
py_receive_mpi_message(PyObject *self, PyObject *args, PyObject *keywds)
{
	int rank;
	int timeout;
	int status;
	PyObject *message;
	int message_size;
	
	static char *kwlist[] = {"rank", "timeout", "message_size", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, keywds, "iii", kwlist,
					&rank, &timeout, &message_size))
		return NULL;

	message = PyString_FromStringAndSize((char *)NULL, message_size);
	if (message == NULL)
		return NULL;

	status = receive_mpi_message(rank, timeout, PyString_AsString(message), message_size);
	if (status < 0) {
		Py_DECREF(message);
		return posix_error();
	}

	_PyString_Resize(&message, status);
	
	return Py_BuildValue("Ni", message, status);
}


static PyMethodDef msgMethods[] = {
    {"send_mpi_message",  (PyCFunction)py_send_mpi_message, METH_VARARGS | METH_KEYWORDS,
     "send_mpi_message(rank, message)\n\nSend the message, message, to the process with the rank, rank.\nReturns - None.\n\nExample:\n> send_mpi_message(rank=12, message='Hello') - will send the message 'Hello' to the process with rank, rank."},
    {"register_mpi",  py_register_mpi, METH_NOARGS,
     "register_mpi()\n\nRegister to the MPI communication list.\n\nReturns - Rank of process.\n\nExample:\n> register_mpi()"},
    {"receive_mpi_message",  (PyCFunction)py_receive_mpi_message, METH_VARARGS | METH_KEYWORDS,
     "receive_mpi_message(rank, timeout, message_size)\n\nReturns the first message received from process with rank, rank. If no message was received from that process the function waits for such message for timeout time. If no such message arrives the function raises exception OSError(EFAULT)\nReturns - A tuple whose first element is the read message and the second is the return value of the underlined system call (presumably the number of copied bytes).\n\nExample:\nreceive_mpi_message(rank=12, timeout=5, message_size=1000)."},
    {NULL, NULL, 0, NULL} 
};


void
initMPI(void)
{
    (void) Py_InitModule("MPI", msgMethods);
}
