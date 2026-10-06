#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>

#define GPIOLED_CNT 1
#define GPIOLED_NAME "gpioled"
#define LED_ON 1
#define LED_OFF 0


struct gpioled_dev
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *dev;
    int major;
    int minor;
    struct device_node *nd;
    int led_gpio;
    struct mutex lock;
}gpioled;

static int led_open(struct inode *inode, struct file *file)
{
    file->private_data = &gpioled;
    if(mutex_lock_interruptible(&gpioled.lock)) {
        return -EINTR;
    }
    return 0;
}

static ssize_t led_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    return 0;
}

ssize_t led_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    int val;
    unsigned char ledbuf[1];
    unsigned char ledstate;
    struct gpioled_dev *dev = file->private_data;

    val = copy_from_user(ledbuf, buf, size);


    if (val < 0) {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }
    ledstate = ledbuf[0];

    if(ledbuf[0] == LED_ON) {
        gpio_set_value(dev->led_gpio, 0);
    } else if(ledbuf[0] == LED_OFF) {
        gpio_set_value(dev->led_gpio, 1);
    } else {
        printk("Invalid LED state!\r\n");
        return -EINVAL;
    }
    return 0;
}

static int led_release(struct inode *inode, struct file *file)
{
    mutex_unlock(&gpioled.lock);
    return 0;
}

struct file_operations gpioled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

static int __init led_init(void)
{
    int ret = 0;
    mutex_init(&gpioled.lock);
    gpioled.nd = of_find_node_by_path("/gpioled");
    if(gpioled.nd == NULL) {
        printk("gpioled node not find!\r\n");
        return -EINVAL;
    }
    else {
        printk("gpioled node find!\r\n");
    }

    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "gpios", 0);
    if(gpioled.led_gpio < 0) {
        printk("can't get led-gpio\r\n");
        return -EINVAL;
    }   
    printk("led-gpio=%d\r\n", gpioled.led_gpio);

    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if(ret < 0) {
        printk("gpioled set gpio value failed!\r\n");
        return -EINVAL;
    }

if(gpioled.major) 
    {
        gpioled.devid = MKDEV(gpioled.major, 0);
        register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
    } 
    else 
    {
        alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);
        gpioled.major = MAJOR(gpioled.devid);
        gpioled.minor = MINOR(gpioled.devid);
    }
    printk("gpioled major=%d, minor=%d\r\n", gpioled.major, gpioled.minor);

    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_fops);

    cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
    if (IS_ERR(gpioled.class)) {
        printk("gpioled class_create failed!\r\n");
        return -1;
    }

    gpioled.dev = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
    if (IS_ERR(gpioled.dev)) {
        printk("gpioled device_create failed!\r\n");
        return -1;
    }

    return 0;
}

static void __exit led_exit(void)
{
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);

    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);

}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lzy");