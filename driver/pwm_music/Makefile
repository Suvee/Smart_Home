ifeq ($(KERNELRELEASE), )
KERNDIR ?= /home/linux/linux-3.14/ 
PWD ?= $(shell pwd)
modules:
	make -C $(KERNDIR) M=$(PWD) modules
clean:
	make -C $(KERNDIR) M=$(PWD) clean
app:
	arm-none-linux-gnueabi-gcc pwm_music.c -Wall -o app 
.PHONY:modules clean app
else
	obj-m += dt_pwm.o
endif
