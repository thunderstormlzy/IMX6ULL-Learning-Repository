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
#include <linux/timer.h>


#define TIMER_CNT 1
#define TIMER_NAME "timer"
#define LEDOFF 0
#define LEDON 1
#define TIMER_IOC 'T'
#define CLOSE_CMD (_IO(TIMER_IOC, 0x1))
#define OPEN_CMD (_IO(TIMER_IOC, 0x2))
#define SETPERIOD_CMD (_IO(TIMER_IOC, 0x3))

struct timer_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;

    struct device_node *nd;
    int led_gpio;
    int timerperiod;
    struct timer_list timer;
    spinlock_t lock;
}timerdev;

static int led_init(void)
{
    int ret = 0;
    timerdev.nd = of_find_node_by_path("/gpioled");
    if (timerdev.nd == NULL) {
        printk("gpioled node not found!\r\n");
        return -EINVAL;
    } else {
        printk("gpioled node has been found!\r\n");
    }

    timerdev.led_gpio = of_get_named_gpio(timerdev.nd, "gpios", 0);
    if (timerdev.led_gpio < 0) {
        printk("can't get led-gpio\r\n");
        return -EINVAL;
    } else {
        printk("led-gpio num = %d\r\n", timerdev.led_gpio);
    }

    ret = gpio_request(timerdev.led_gpio, "gpioled");
    if (ret) {
        printk("failed to request led-gpio\r\n");
        return -EINVAL;
    }

    ret = gpio_direction_output(timerdev.led_gpio, 1);
    if (ret) {
        printk("failed to set led-gpio as output\r\n");
        return -EINVAL;
    }

    return 0;
}

static int timer_open(struct inode *inode, struct file *file)
{
    file->private_data = &timerdev;
    return 0;
}

static long timer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct timer_dev *dev = (struct timer_dev *)file->private_data;
    int timerperiod;
    unsigned long flags;

    switch (cmd) {
    case OPEN_CMD:
        spin_lock_irqsave(&dev->lock, flags);
        timerperiod = dev->timerperiod;
        spin_unlock_irqrestore(&dev->lock, flags);
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(timerperiod));
        break;
    case CLOSE_CMD:
        del_timer_sync(&dev->timer);
        break;
    case SETPERIOD_CMD:
        spin_lock_irqsave(&dev->lock, flags);
        dev->timerperiod = arg;
        spin_unlock_irqrestore(&dev->lock, flags);
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->timerperiod));
        break;
    default:
        break;
    }
    return 0;
}

struct file_operations timer_fops = {
    .owner = THIS_MODULE,
    .open = timer_open,
    .unlocked_ioctl = timer_ioctl,
};

void timer_function(unsigned long data)
{
    struct timer_dev *dev = (struct timer_dev *)data;
    unsigned long flags;
    static int state=1;
    int timerperiod;

    state =!state;
    gpio_set_value(dev->led_gpio, state);

    spin_lock_irqsave(&dev->lock, flags);
    timerperiod = dev->timerperiod;
    spin_unlock_irqrestore(&dev->lock, flags);
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->timerperiod));
}

static int __init timer_init(void)
{
    int ret = 0;
    timerdev.timerperiod = 1000;
    led_init();

    spin_lock_init(&timerdev.lock);
    
    if(timerdev.major) {
        timerdev.devid = MKDEV(timerdev.major, 0);
        ret = register_chrdev_region(timerdev.devid, TIMER_CNT, TIMER_NAME);
    } else {
        ret = alloc_chrdev_region(&timerdev.devid, 0, TIMER_CNT, TIMER_NAME);
        timerdev.major = MAJOR(timerdev.devid);
        timerdev.minor = MINOR(timerdev.devid);
    }

    timerdev.cdev.owner = THIS_MODULE;
    cdev_init(&timerdev.cdev, &timer_fops);
    cdev_add(&timerdev.cdev, timerdev.devid, TIMER_CNT);

    timerdev.class = class_create(THIS_MODULE, TIMER_NAME);
    if (IS_ERR(timerdev.class)) {
        return PTR_ERR(timerdev.class);
    }

    timerdev.device = device_create(timerdev.class, NULL, timerdev.devid, NULL, TIMER_NAME);
    if (IS_ERR(timerdev.device)) {
        return PTR_ERR(timerdev.device);
        
    }

    init_timer(&timerdev.timer);
    timerdev.timer.function = timer_function;
    timerdev.timer.data = (unsigned long)&timerdev;

    return 0;
}

static void __exit timer_exit(void)
{
    gpio_set_value(timerdev.led_gpio, 1);
    gpio_free(timerdev.led_gpio);
    del_timer_sync(&timerdev.timer);

    device_destroy(timerdev.class, timerdev.devid);
    class_destroy(timerdev.class);
    cdev_del(&timerdev.cdev);
    unregister_chrdev_region(timerdev.devid, TIMER_CNT);
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lzy");