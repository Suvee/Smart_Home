#ifndef __SHM_COM__
#define __SHM_COM__

#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>

#define SIZE 10

typedef char data_t;
typedef struct shm_data {
	pid_t pid;
	data_t buf[SIZE];
}shm_data_t;

void signal_handle(int sig);

void signal_handle(int sig)
{ /*nothing*/ }

#endif
