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
static char recv_buff[NUM];
/*file descriptor on /dev/ttyUSB0 */
int fd_serial;

/*sensor data*/
char temperature[2];
char humidity[2];
char gas[2];

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

/*
 * Name      : serial_data_handle()
 * Function  : recive and handle serial recive data
 * Parameter : buffer container of recive
 * Return    : void 
 * */
static int serial_data_handle(void)
{
	int i = 0;
	const char *tmp = recv_buff;

	if ((0xFE==tmp[0]) && (0xEF==tmp[1]) && (0xFF==tmp[8])) {
		gas[0] = tmp[2];  //high 8 bits
		gas[1] = tmp[3];  //low 8 bits

		temperature[0] = tmp[4];
		temperature[1] = tmp[5];

		humidity[0] = tmp[6];
		humidity[1] = tmp[7];
	} else {
		return -1;
	}

#if 0
	printf("gas:%.2f%%, temp:%d.%d, humi:%d.%d\n", 
			gas_trans((gas[0]<<8)|gas[1]),
			temperature[0], temperature[1],
			humidity[0], humidity[1]);
#endif

	return 0;
}

void *pth_uart_func(void)
{
	serial_init (fd_serial);
	
	while (1)
	{	
		bzero(recv_buff, sizeof (recv_buff));

		if (0 < read(fd_serial, recv_buff, sizeof (recv_buff)))
			serial_data_handle();

		sleep(1);
	}
	
	close(fd_serial);

	return 0;
}
