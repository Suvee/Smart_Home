.PHONY:all clean

all:main.o pth_uart.o pth_pwm.o
	arm-none-linux-gnueabi-gcc -Wall main.c pth_uart.c pth_pwm.c -o main -lpthread

clean:
	@rm *.o
	@echo Clean Done.
