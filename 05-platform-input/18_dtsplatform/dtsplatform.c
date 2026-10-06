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

#define LEDDEV_CNT 1
#define LEDDEV_NAME "dtsplatform"
#define LEDOFF 0
#define LEDON 1

struct leddev_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *dev;
    int major;
    struct device_node *node;
    int led0;
}leddev;

void led0_switch(u8 state)
{
    if(state == LEDON)
    {
        gpio_set_value(leddev.led0, 0);
    }
    else if(state == LEDOFF)
    {
        gpio_set_value(leddev.led0, 1);
    }
}

int led_open(struct inode *inode, struct file *file)
{
    file->private_data = &leddev;
    return 0;
}

static ssize_t led_writer(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    unsigned char databuf[2];
    unsigned char ledstate;

    ret=copy_from_user(databuf, buf, count);
    if(ret<0)
    {
        return -EFAULT;
    }

    ledstate = databuf[0];

    if(ledstate == LEDON)
    {
        led0_switch(LEDON);
    }
    else if(ledstate == LEDOFF)
    {
        led0_switch(LEDOFF);
    }

    return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_writer,
};

static int led_probe(struct platform_device *pdev)
{
    printk("led device and driver has matched!\n");

    if(leddev.major)
    {
        leddev.devid = MKDEV(leddev.major, 0);
        register_chrdev_region(leddev.devid, LEDDEV_CNT, LEDDEV_NAME);
    }
    else
    {
        alloc_chrdev_region(&leddev.devid, 0, LEDDEV_CNT, LEDDEV_NAME);
        leddev.major = MAJOR(leddev.devid);
    }

    leddev.cdev.owner = THIS_MODULE;
    cdev_init(&leddev.cdev, &led_fops);

    cdev_add(&leddev.cdev, leddev.devid, LEDDEV_CNT);

    leddev.class = class_create(THIS_MODULE, LEDDEV_NAME);
    if(IS_ERR(leddev.class))
    {
        return PTR_ERR(leddev.class);
    }

    leddev.dev = device_create(leddev.class, NULL, leddev.devid, NULL, LEDDEV_NAME);
    if(IS_ERR(leddev.dev))
    {
        return PTR_ERR(leddev.dev);
    }

    leddev.node = of_find_node_by_path("/gpioled");
    if(leddev.node == NULL)
    {
        printk("gpioled node not find!\r\n");
        return -EINVAL;
    }

    leddev.led0 = of_get_named_gpio(leddev.node, "gpios", 0);
    if(leddev.led0 < 0)
    {
        printk("led0 gpio not find!\r\n");
        return -EINVAL;
    }

    gpio_request(leddev.led0, "led0");
    gpio_direction_output(leddev.led0, 1);

    return 0;
}

static int led_remove(struct platform_device *pdev)
{ 
    gpio_set_value(leddev.led0, 1);
    gpio_free(leddev.led0);

    device_destroy(leddev.class, leddev.devid);
    class_destroy(leddev.class);
    cdev_del(&leddev.cdev);
    unregister_chrdev_region(leddev.devid, LEDDEV_CNT);

    return 0;
}

static const struct of_device_id led_of_match[] = {
    { .compatible = "gpio-led" },

};

static struct platform_driver led_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "led",
        .of_match_table = led_of_match,
    },
};

static int __init platform_driver_init(void)
{
    return platform_driver_register(&led_driver);
}

static void __exit platform_driver_exit(void)
{
    platform_driver_unregister(&led_driver);
}

module_init(platform_driver_init);
module_exit(platform_driver_exit);
MODULE_LICENSE("GPL");    
MODULE_AUTHOR("lzy");