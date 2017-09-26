#ifndef __FS4412_COM__
#define __FS4412_COM__

#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

#define SIZE 10
void signal_handle(int sig);

typedef char data_t;
typedef struct shm_data {
	int flag;
	pid_t pid;
	data_t buf[SIZE];
}shm_data_t;

/************************** pth_uart.c ******************************/
extern int fd_serial;
extern char temperature[2];
extern char humidity[2];
extern char gas[2];

/* serial handle pthread function */
extern void *pth_uart_func(void);

/* 
 * sig_beep[2]: beep control flag 
 * 0xf1: Enable beep / 0xf0 Disable beep
 * write (fd_serial, &sig_beep[0], 1);			
 * write (fd_serial, &sig_beep[1], 1);			
 **/
char sig_beep[2] = {0xf1, 0xf0};
/**************************** end ***********************************/

/************************** pth_pwm.c *******************************/
extern int fd_pwm;

/* pwm handle pthread function */
extern void *pth_pwm_func(void);




#endif
