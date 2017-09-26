#include <stdio.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include "shm_common.h"

void signal_handle(int sig)
{ /*nothing*/ }

static char rxbuf[SIZE];
static char recv_buf[128];

int main(int argc, const char *argv[])
{
	int shmid = 0;
	shm_data_t *shmaddr = NULL;
	int peer_id = 0;

	signal(SIGUSR1, signal_handle);
	
#if 0
	if ((fd_serial = open ("/dev/ttyUSB0", O_RDWR)) < 0)
	{
		perror ("open ttyUSB");
		return -1;
	}
	printf ("serial_init is ok, open /dev/ttyUSB0 success!\n");
#endif

	key_t key = ftok("/", 130);
	if (-1 == key) {
		perror("fail to ftok\n");
		strerror(errno);
		return -1;
	}

	shmid = shmget(key, sizeof (shm_data_t), IPC_CREAT|IPC_EXCL|0666);
	if (-1 == shmid) {
		if (EEXIST == errno) {
			shmid = shmget(key, sizeof (shm_data_t), 0666);
			shmaddr = shmat(shmid, NULL, 0);

			peer_id = shmaddr->pid;
			shmaddr->pid = getpid();
			kill(peer_id, SIGUSR1);
		} else {
			perror("fail to shmget\n");
			strerror(errno);
			return -1;
		}
	} else {
		shmaddr = shmat(shmid, NULL, 0);
		if ((void *)-1 == shmaddr) {
			perror("fail to shmat\n");
			strerror(errno);
			return -1;
		}
		shmaddr->pid = getpid();
		
		pause();
	}

	int rd_byte = 0;

	while(1)
	{
		bzero(shmaddr->buf, sizeof (shmaddr->buf));
		rd_byte = read(0, shmaddr->buf, sizeof (shmaddr->buf));
		if (-1 == rd_byte) { 			//error situation
			perror("fail to read\n");
			strerror(errno);
			return -1;
		} else if (1 == rd_byte) {//end-of-file
			printf("end-of-file\n");
			kill(peer_id, SIGUSR1);
			break;
		} else { 						//normal
			peer_id = shmaddr->pid;
			shmaddr->pid = getpid();
			kill(peer_id, SIGUSR1);
		}

		pause();
	}

	shmdt(shmaddr);
	
	return 0;
}
