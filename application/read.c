#include <stdio.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "shm_common.h"

int main(int argc, const char *argv[])
{
	int shmid = 0;
	shm_data_t *shmaddr = NULL;
	int peer_id = 0;

	signal(SIGUSR1, signal_handle);
	
	key_t key = ftok("/", 'a');
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

	int wr_byte = 0;

	while(1)
	{
		pause();

		wr_byte = write(1, shmaddr->buf, sizeof (shmaddr->buf));
		if (-1 == wr_byte) { 			//error situation
			perror("fail to read\n");
			strerror(errno);
			return -1;
		} else if (!strncmp("\n", shmaddr->buf, 1)) {//end-of-file
			printf("end-of-file\n");
			break;
		} else { 						//normal
			peer_id = shmaddr->pid;
			shmaddr->pid = getpid();
			kill(peer_id, SIGUSR1);
		}
	}

	shmdt(shmaddr);
	
	return 0;
}
