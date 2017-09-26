#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "pwm_music.h"
#include "fs4412_common.h"

/*file descriptor on pwm_beep*/
int fd_pwm;

/*share memery address on pthread music*/
music_data_t *music_shm_addr;
/*share memery id*/
int music_shmid;

/*
 * Name      : play_music(number)
 * Function  : Playing specified song accroding to number 
 * Parameter : num --> switch songs
 * Note      : Write 1/2/3 to chose corresponding song,
 * 			   if the num byend of this range, chose default 1.
 * 			   and when you wanna stop, write -1 to it.
 * Return    : void
 * */
static void play_music(int num)
{
	int i = 0;
	int div = 0;
	int pre = 255;

	ioctl(fd_pwm, PWM_ON);
	ioctl(fd_pwm, SET_PRE, &pre);

	switch (num)
	{
	default:
		/*default to play No.1 song*/
	case 1:
		for(i = 0; i<sizeof(MumIsTheBestInTheWorld)/sizeof(Note); i++)
		{
			div = (PCLK/256/4)/(MumIsTheBestInTheWorld[i].pitch);
			ioctl(fd_pwm, SET_CNT, &div);
			usleep(MumIsTheBestInTheWorld[i].dimation * 50); 
		}
		break;

	case 2:
		for(i = 0; i<sizeof(GreatlyLongNow)/sizeof(Note); i++)
		{
			div = (PCLK/256/4)/(GreatlyLongNow[i].pitch);
			ioctl(fd_pwm, SET_CNT, &div);
			usleep(GreatlyLongNow[i].dimation * 50); 
		}
		break;

	case 3:
		for(i = 0; i<sizeof(FishBoat)/sizeof(Note); i++)
		{
			div = (PCLK/256/4)/(FishBoat[i].pitch);
			ioctl(fd_pwm, SET_CNT, &div);
			usleep(FishBoat[i].dimation * 50); 
		}
		break;

	case -1:
		break;
	}

	ioctl(fd_pwm, PWM_OFF);
}

/*
 * Name      : pth_pwm_func(void)
 * Function  : pwm beep pthread function 
 * Parameter : void
 * Return    : error messages
 * */
void *pth_pwm_func(void)
{
	char *errmsg;

	/****************** share memery create *******************/
	key_t key = ftok("/", 128);
	if (-1 == key) {
		errmsg = strerror(errno);
		return errmsg;
	}

	music_shmid = shmget(key, sizeof (music_data_t), IPC_CREAT|IPC_EXCL|0666);
	if (-1 == music_shmid) {
		if (EEXIST == errno) {
			music_shmid = shmget(key, sizeof (music_data_t), 0666);
			music_shm_addr = shmat(music_shmid, NULL, 0);
			music_shm_addr->flag = 0; //stop play music
		} else {
			errmsg = strerror(errno);
			return errmsg;
		}
	} else {
		music_shm_addr = shmat(music_shmid, NULL, 0);
		if ((void *)-1 == music_shm_addr) {
			errmsg = strerror(errno);
			return errmsg;
		}

		music_shm_addr->flag = 0; //stop play music
	}

	while (1) 
	{
		if (1 == music_shm_addr->flag) {
			play_music(music_shm_addr->num);
		} else {
			play_music(-1);
		}

		sleep(2);
	}

	pthread_exit(NULL);
}

int main(int argc, const char *argv[])
{
	
	return 0;
}
