#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include<linux/miscdevice.h>

#define MISCBEEP_NAME "miscbeep"
#define MISCBEEP_MINOR 144
#define BEEPOFF 0
#define BEEPON 1

struct miscbeep_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *dev;
    struct device_node *nd;
    int beep_gpio;
}miscbeep;

static int miscbeep_open(struct inode *inode, struct file *file)
{
    file->private_data = &miscbeep;
    return 0;
}

static ssize_t miscbeep_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    unsigned char databuf[1];
    unsigned char beepstate;
    struct miscbeep_dev *dev = file->private_data;

    ret=copy_from_user(databuf, buf, count);
    if(ret<0)
    {
        printk("kernel copy_from_user failed\n");
        return -EFAULT;
    }

    beepstate=databuf[0];
    if(beepstate==BEEPOFF)
    {
        gpio_set_value(dev->beep_gpio, 1);
    }
    else if(beepstate==BEEPON)
    {
        gpio_set_value(dev->beep_gpio, 0);
    }
    return 0;
}

static struct file_operations miscbeep_fops = {
    .owner = THIS_MODULE,
    .open = miscbeep_open,
    .write = miscbeep_write,
};

static struct miscdevice beep_miscdev = {
    .minor = MISCBEEP_MINOR,
    .name = MISCBEEP_NAME,
    .fops = &miscbeep_fops,
};

static int miscbeep_probe(struct platform_device *pdev)
{
    int ret=0;

    printk("beep driver and device was matched!\r\n");

    miscbeep.nd=of_find_node_by_path("/gpio_beep");
    if(miscbeep.nd==NULL)
    {
        printk("beep device not found!\r\n");
        return -ENODEV;
    }

    miscbeep.beep_gpio=of_get_named_gpio(miscbeep.nd, "gpios", 0);
    if(miscbeep.beep_gpio<0)
    {
        printk("gpios not found!\r\n");
        return -EINVAL;
    }

    gpio_request(miscbeep.beep_gpio, "beep");
    ret=gpio_direction_output(miscbeep.beep_gpio, 1);
    if(ret<0)
    {
        printk("gpio_direction_output failed!\r\n");
        return -EINVAL;
    }

    ret=misc_register(&beep_miscdev);
    if(ret<0)
    {
        printk("misc_register failed!\r\n");
        return -EFAULT;
    }
    return 0;
}

static int miscbeep_remove(struct platform_device *pdev)
{
    int ret=0;

    gpio_set_value(miscbeep.beep_gpio, 1);
    gpio_free(miscbeep.beep_gpio);

    ret=misc_deregister(&beep_miscdev);
    if(ret<0)
    {
        printk("misc_deregister failed!\r\n");
        return -EFAULT;
    }

    printk("beep driver and device was removed!\r\n");
    return 0;
}

static const struct of_device_id miscbeep_of_match[] = {
    { .compatible = "gpio-beep", },
    {},
    {},
};

static struct platform_driver miscbeep_driver = {
    .driver = {
        .name = "miscbeep",
        .of_match_table = miscbeep_of_match,
    },
    .probe = miscbeep_probe,
    .remove = miscbeep_remove,
};

static int __init miscbeep_init(void)
{
    return platform_driver_register(&miscbeep_driver);
}

static void __exit miscbeep_exit(void)
{
    platform_driver_unregister(&miscbeep_driver);
}

module_init(miscbeep_init);
module_exit(miscbeep_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lzy");