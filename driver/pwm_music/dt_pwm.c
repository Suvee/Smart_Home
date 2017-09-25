/*
 * In this module, use device tree to match device information.
 * but it's not based on platform bus, so when you get resource
 * from exynos4412-fs4412.dts, don't use (platform_get_resource)
 * function. 
 * 
 * Just use dev->resource[i].start and dev->resource[i].end
 * In device tree, reg = <0x114000a0 0x4>
 * means .start = 0x114000a0, .end = .start + 0x4
 *
 * */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include "s5pc100_pwm.h"

#define DEV_NAME "pwm_music"
#define CLS_NAME "xpwm"
#define DEV_NUM 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Suvee");
MODULE_DESCRIPTION("Driver for pwm beep");
MODULE_VERSION("v1.0.0");

static struct class *cls_p;

static int major;

/*pwm beep register: GPD0_0 */
volatile unsigned int *gpd0con;
/*8BIT PRESCALER 0*/
volatile unsigned int *tcfg0; 	
/*PRESCALER 1, level: 1/1. 1/2. 1/4. 1/8. 1/16. */
volatile unsigned int *tcfg1;
/*Specifies timer control register*/
volatile unsigned int *tcon;
/*Timer0 count buffer register*/
volatile unsigned int *tcntb0;
/*Timer0 compare buffer register*/
volatile unsigned int *tcmpb0;


static int pwm_open(struct inode *inode, struct file *file)
{
	printk("func_open /dev/pwm%d success!\n", iminor(inode));
	return 0;
}
static long pwm_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long argu)
{
	int val;

	switch (cmd) {
	case SET_PRE:
		if (copy_from_user(&val, (int __user *)argu, sizeof(int)))
			return -EFAULT;
		/* set prescaler0: 100M/val */
		writel((readl(tcfg0) | (val << 0)), tcfg0);
		break;

	case SET_CNT:
		if (copy_from_user(&val, (int __user *)argu, sizeof(int)))
			return -EFAULT;

		/*set Timer0 count register: val*/
		writel((val << 0), tcntb0);
		/*Timer0 compare register: val/2 ---> Duty cycle:1/2*/
		writel((readl(tcntb0) >> 1), tcmpb0);

		break;

	case PWM_ON:
		/*tcon[0]:start/stop timer, tcon[3]:aotu-reload on/off*/
		writel((readl(tcon) & ~(0xf << 0)) | (0x9 << 0), tcon);
		break;

	case PWM_OFF:
		writel((readl(tcon) & ~(0xf << 0)) | (0x8 << 0), tcon);
		break;

	default:
		printk("error\n");
		return -EINVAL;
		break;
	}

	return 0;
}

struct file_operations pwm_fops = {
	.owner = THIS_MODULE,
	.open  = pwm_open,
	.unlocked_ioctl = pwm_unlocked_ioctl,
};

static int pwm_probe(struct platform_device *dev)
{
	int ret;

	printk("probe ok!\n");
	major = register_chrdev(0, DEV_NAME, &pwm_fops);
	if (0 > major) {
		printk("register_chrdev fail!, errno:%d\n", major);
		return major;
	} else
		printk("register_chrdev success! major:%d\n", major);

	cls_p = class_create(THIS_MODULE, CLS_NAME);
	if (NULL == cls_p) {
		printk("class_create fail!\n");
		ret = -ENXIO;
		goto err1;
	} else
		printk("class_create success!\n");

	/*operational device node: /dev/pwm */
	device_create(cls_p, NULL, major << 20 | 0, NULL, "pwm");

	gpd0con = ioremap(dev->resource[0].start, 
			dev->resource[0].end - dev->resource[0].start);
	if (NULL == gpd0con) {
		ret = -ENOMEM;
		goto err2;
	}
	tcfg0 = ioremap(dev->resource[1].start, 
			dev->resource[1].end - dev->resource[1].start);
	if (NULL == tcfg0) {
		ret = -ENOMEM;
		goto err3;
	}

							//tcfg0  = 0x139D0000
	tcfg1  = tcfg0 + 1; 	//tcfg1  = 0x139D0004
	tcon   = tcfg0 + 2; 	//tcon   = 0x139D0008 
	tcntb0 = tcfg0 + 3; 	//tcntb0 = 0x139D000C
	tcmpb0 = tcfg0 + 4; 	//tcmpb0 = 0x139D0010

	//0x2:TOUT_0
	writel((readl(gpd0con) & ~(0xf << 0)) | (0x2 << 0), gpd0con);
	//prescaler0 = 255
	writel((readl(tcfg0) | (0xff << 0)), tcfg0);
	//prescaler1 = 1/2
	writel((readl(tcfg1) & ~(0xf << 0)) | (0x1 << 0), tcfg1);
	//count register = 0x500
	writel((readl(tcntb0) & ~(0xffff)) | (0x500 << 0), tcntb0);
	//compare register = tcntb >> 1 == 0x280
	writel((readl(tcmpb0) & ~(0xffff)) | (0x280 << 0), tcmpb0);
	//tcon[0]:stop timer, tcon[1]: update tcntb0 and tcmpb0
	writel((readl(tcon) & ~(0xf << 0)) | (0x2 << 0), tcon);

	return 0;

err3:
	iounmap(gpd0con);
err2:
	class_destroy(cls_p);
err1:
	unregister_chrdev(major, DEV_NAME);
	return ret;
}

static int pwm_remove(struct platform_device *dev)
{
	printk("remove done!\n");

	iounmap(tcfg0);
	iounmap(gpd0con);

	device_destroy(cls_p, major << 20 | 0);

	class_destroy(cls_p);

	unregister_chrdev(major, DEV_NAME);

	return 0;
}

static struct of_device_id pwm_match_table[] = {
	{ .compatible = "fs4412, pwm", },
	{/*nothing*/},
};
static struct platform_driver pwm_drevie = {
	.probe  = pwm_probe,
	.remove = pwm_remove,
	.driver = {
		.name = "fs4412-pwm",
		.of_match_table = pwm_match_table,
	},
};

static int __init pwm_init(void)
{	 
	platform_driver_register(&pwm_drevie);

	return 0;
}

static void __exit pwm_exit(void)
{
	platform_driver_unregister(&pwm_drevie);
}

module_init(pwm_init);
module_exit(pwm_exit);
