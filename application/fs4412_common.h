#ifndef __FS4412_COM__
#define __FS4412_COM__

#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>

#define SIZE 10

/************************** pth_uart.c ******************************/
extern int fd_serial;

typedef struct uart_shm_data {
	/*read or write flag: 0 to read / 1 to write*/
	unsigned int flag;
	/*sensor data*/
	char temperature[2];
	char humidity[2];
	char gas[2];
}uart_data_t;

extern uart_data_t *uart_shm_addr;
extern int uart_shmid;

/* serial handle pthread function */
extern void *pth_uart_func(void);

/* 
 * sig_beep[2]: beep control flag 
 * 0xf1: Enable beep / 0xf0 Disable beep
 * write (fd_serial, &sig_beep[0], 1);			
 * write (fd_serial, &sig_beep[1], 1);			
 **/
extern char sig_beep[2];
/**************************** end ***********************************/

/************************** pth_pwm.c *******************************/
extern int fd_pwm;

typedef struct music_shm_data {
	/*read or write flag: 0 to read / 1 to write*/
	unsigned int flag;
	/*which song to play*/
	int num;
}music_data_t;

extern music_data_t *music_shm_addr;
extern int music_shmid;

/* pwm handle pthread function */
extern void *pth_pwm_func(void);


#endif
