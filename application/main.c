#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include "fs4412_common.h"

static pthread_t pth_uart;
static pthread_t pth_pwm;

int main(int argc, const char *argv[])
{
	int err = 0;

	fd_serial = open("/dev/ttyUSB0", O_RDWR); 
	if (-1 == fd_serial)
		goto error1;
	fd_pwm = open("/dev/pwm", O_RDWR | O_NONBLOCK);
	if (-1 == fd_pwm)
		goto error1;

	err = pthread_create(&pth_uart, NULL, pth_uart_func, NULL);
	if (err)
		goto error1;
	err = pthread_create(&pth_pwm, NULL, pth_pwm_func, NULL);
	if (err)
		goto error1;

	err = pthread_join(pth_uart, NULL);
	if (err)
		goto error1;
	err = pthread_join(pth_pwm, NULL);
	if (err)
		goto error1;


	printf("never go here!\n");


	close(fd_serial);
	close(fd_pwm);

	return 0;

error1:
	strerror(errno);
	return err;
}
