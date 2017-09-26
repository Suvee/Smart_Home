#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include "fs4412_common.h"

#define NUM 9

/*recive STM32 sampling informations */
static unsigned char recv_buff[NUM];
/*file descriptor on /dev/ttyUSB0 */
int fd_serial;
/*share memery address*/
uart_data_t *uart_shm_addr;
/*share memery id*/
int uart_shmid;

/*signal to control beep*/
char sig_beep[2] = {0xf1, 0xf0};

/*
 * Name      : serial_init()
 * Parameter : file descriptor(fd)
 * Function  : initialize UART1
 * Local     : serial pthread
 * Return    : void
 * */
static void serial_init(int fd)
{
	//termios数族提供一个常规的终端接口，用于控制 非同步 通信端口。
	struct termios options;

	//取得当前串口的属性，并付给 fd 这个设备
	tcgetattr(fd, &options);

	/***********************  c_cflag  ***************************/
	//CLOCAL:程序不占用端口，CREAD:使能读取数据
	options.c_cflag |= (CLOCAL | CREAD);
	//清空原有 字符大小位
	options.c_cflag &= ~CSIZE;
	//设置字符长度为 8bit
	options.c_cflag |= CS8;
	//不使用流控制
	options.c_cflag &= ~CRTSCTS;
	//1 bit停止位( |= CSTOPB: 表示设置两个停止位)
	options.c_cflag &= ~CSTOPB;

	/***********************  c_iflag  ***************************/
	//忽略帧错误和奇偶校验错误
	options.c_iflag |= IGNPAR;
 	// 中断条件|奇偶校验|去除最高位|回车转换为换行|tty输出停止控制
	options.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);

	/**********************  other flag  *************************/
	//最小字符长度 : 0
	options.c_cc[VMIN] = 0;
	//设置输出模式标志
	options.c_oflag = 0;
	//设置本地模入标志
	options.c_lflag = 0;

	cfsetispeed(&options,B115200);//设置输入波特率: 115200
	cfsetospeed(&options,B115200);//设置输出波特率: 115200

	//将上述设定的属性赋值给 fd 设备. TCSANOW: 马上生效
	tcsetattr(fd, TCSANOW,&options);

	printf("init gprs over...\n");
	return ;
}

#if 0
/*
 * Name      : gas_trans()
 * Function  : handle data from STM32, 
 * 			   translate AD value to gas denseness %  
 * Parameter : AD value 
 * */
static float gas_trans(int t)
{
	return ((float)t / 4096 * 100);
}
#endif

/*
 * Name      : serial_data_handle()
 * Function  : recive and handle serial recive data
 * Parameter : buffer container of recive
 * Return    : void 
 * */
static void serial_data_handle(void)
{
	unsigned char *tmp = recv_buff;

	if ((0xFE==tmp[0]) && (0xEF==tmp[1]) && (0xFF==tmp[8])) {
		printf("read data ok!\n");

		uart_shm_addr->gas[0] = tmp[2];  //high 8 bits
		uart_shm_addr->gas[1] = tmp[3];  //low 8 bits

		uart_shm_addr->temperature[0] = tmp[4];
		uart_shm_addr->temperature[1] = tmp[5];

		uart_shm_addr->humidity[0] = tmp[6];
		uart_shm_addr->humidity[1] = tmp[7];

		/*set flag to wait Qt read*/
		uart_shm_addr->flag = 1; //wait Qt read
	} 

#if 0
	printf("gas:%.2f%%, temp:%d.%d, humi:%d.%d\n", 
			gas_trans((uart_shm_addr->gas[0]<<8)|uart_shm_addr->gas[1]),
			uart_shm_addr->temperature[0], uart_shm_addr->temperature[1],
			uart_shm_addr->humidity[0], uart_shm_addr->humidity[1]);
#endif

	return ;
}

void *pth_uart_func(void *pth)
{
	char *errmsg;

	serial_init(fd_serial);

	/****************** share memery create *******************/
	key_t key = ftok("/", 130);
	if (-1 == key) {
		errmsg = strerror(errno);
		return errmsg;
	}

	uart_shmid = shmget(key, sizeof (uart_data_t), IPC_CREAT|IPC_EXCL|0666);
	if (-1 == uart_shmid) {
		if (EEXIST == errno) {
			uart_shmid = shmget(key, sizeof (uart_data_t), 0666);
			uart_shm_addr = (uart_data_t *)shmat(uart_shmid, NULL, 0);
			uart_shm_addr->flag = 0; //sign to read
		} else {
			errmsg = strerror(errno);
			return errmsg;
		}
	} else {
		uart_shm_addr = (uart_data_t *)shmat(uart_shmid, NULL, 0);
		if ((void *)-1 == uart_shm_addr) {
			errmsg = strerror(errno);
			return errmsg;
		}

		uart_shm_addr->flag = 0; //sign to read
	}

	/**************** serial data recv/send ******************/
	while (1)
	{	
		bzero(recv_buff, sizeof (recv_buff));

		if (0 == uart_shm_addr->flag) {
			if (0 < read(fd_serial, recv_buff, sizeof (recv_buff))) {
				serial_data_handle();
			}
		}

		if (0x0950 < ((uart_shm_addr->gas[0]<<8) | uart_shm_addr->gas[1]))
			write (fd_serial, &sig_beep[0], 1);			

		usleep(200000);

		if (2 == uart_shm_addr->flag)
			break; //shutdown
	}

	shmdt(uart_shm_addr);

	pthread_exit(NULL);
}
