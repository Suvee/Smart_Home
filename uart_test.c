#include <stdio.h>
#include <stdio.h>
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

#define NUM 10
char rxbuf[NUM];
char recv_buf[128];

void serial_init(int fd)
{
	struct termios options;//termios数族提供一个常规的终端接口，用于控制非同步通信端口。
	tcgetattr(fd, &options);//取得当前串口的属性，并付给collect_fd这个设备
	options.c_cflag |= (CLOCAL | CREAD);//clocal表示å½略modem控制线，cread表示打开接收者
	options.c_cflag &= ~CSIZE;//清空原有字符长度（csize表示原有字符长度掩码）
	options.c_cflag &= ~CRTSCTS;//启用RTS/CTS（硬件）流控制
	options.c_cflag |= CS8;//设置字符长度掩码
	options.c_cflag &= ~CSTOPB;//停止位为1个（cstopb表示设置两个停止位）
	options.c_iflag |= IGNPAR;//忽略帧错误和奇偶校验错
	options.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);

	options.c_cc[VMIN] = 0;//字符长度

	options.c_oflag = 0;//设置输出模式标志
	options.c_lflag = 0;//设置本地模入标志

	cfsetispeed(&options,B115200);//设置输入波特率
	cfsetospeed(&options,B115200);//设置输出波特率

	tcsetattr(fd, TCSANOW,&options);//把上面设置号的属性赋值给collect_fd这个设备，tcsanow表示马上生效
	printf("init gprs over...\n");
	return ;
}

float gas_trans(int t)
{
	return ((float)t / 4096 * 100);
}

void serial_data_handle(void)
{
	const char *p = recv_buf;
	char temperature[2] = {0};
	char humidity[2] = {0};
	char gas[2] = {0};

	int i;

	for (i = 0; i < NUM; i ++) {
		switch (p[i])
		{
		case 'g':
			gas[0] = p[i+1]; 	//high 8 bits
			gas[1] = p[i+2];  //low 8 bits
			break;
		case 't':
			temperature[0] = p[i+1];
			temperature[1] = p[i+2];
			break;
		case 'h':
			humidity[0] = p[i+1];
			humidity[2] = p[i+2];
			break;
		case 's':
			bzero(recv_buf, sizeof (recv_buf));
			break;
		default:
			break;
		}
	}

	printf("gas:%f%%, temp:%d.%d, humi:%d.%d\n", 
			gas_trans((gas[0]<<8)|gas[1]),
			temperature[0], temperature[1],
			humidity[0], humidity[1]);

	return;
}

int main(int argc, char *argv[])
{

	int ret;
	int dev_uart_fd;
	char c[2] = {0xf1, 0xf0};
#if 1
	if ((dev_uart_fd = open ("/dev/ttyUSB0", O_RDWR)) < 0)
	{
		perror ("open ttyUSB");
		return -1;
	}
	serial_init (dev_uart_fd);
	printf ("serial_init is ok\n");
	printf ("pthread_uart_recv is ok\n");

#endif

	while (1)
	{			
		sleep(1);
		memset (recv_buf, 0, sizeof (recv_buf));

		//write (dev_uart_fd, &c[1], 1);			
		ret = read (dev_uart_fd,recv_buf, sizeof(recv_buf));	
		if(ret > 0)
		{
			//printf("recv_buf=%s\n",recv_buf);		
			serial_data_handle();
		}
	}
	close(dev_uart_fd);
	return 0;
}
