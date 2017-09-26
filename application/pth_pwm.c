#include <sys/ioctl.h>
#include "pwm_music.h"
#include "s5pc100_pwm.h"

int fd_pwm;

void *pth_pwm_func(void)
{
	int i = 0;
	int div = 0;
	int pre = 255;

	ioctl(fd_pwm, PWM_ON);
	ioctl(fd_pwm, SET_PRE, &pre);
	for(i = 0; i<sizeof(MumIsTheBestInTheWorld)/sizeof(Note); i++)
	{
		div = (PCLK/256/4)/(MumIsTheBestInTheWorld[i].pitch);
		ioctl(dev_fd, SET_CNT, &div);
		usleep(MumIsTheBestInTheWorld[i].dimation * 50); 
	}

	for(i = 0;i<sizeof(GreatlyLongNow)/sizeof(Note);i++ )
	{
		div = (PCLK/256/4)/(GreatlyLongNow[i].pitch);
		ioctl(dev_fd, SET_CNT, &div);
		usleep(GreatlyLongNow[i].dimation * 50); 
	}

	for(i = 0;i<sizeof(FishBoat)/sizeof(Note);i++ )
	{
		div = (PCLK/256/4)/(FishBoat[i].pitch);
		ioctl(dev_fd, SET_CNT, &div);
		usleep(FishBoat[i].dimation * 50); 
	}
	ioctl(dev_fd,PWM_OFF);

	return 0;
}
