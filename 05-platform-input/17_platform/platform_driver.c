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
#define LEDDEV_NAME "platform_led"
#define LEDOFF 0
#define LEDON 1

static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *IMX6U_MUX_GPIO1_IO03;
static void __iomem *IMX6U_PAD_GPIO1_IO03;
static void __iomem *IMX6U_GPIO1_DR;
static void __iomem *IMX6U_GPIO1_GDIR;

struct leddev_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *dev;
    int major;
}leddev;

void led0_switch(u8 state)
{
    unsigned int val;

    if(state == LEDON)
    {
        val = readl(IMX6U_GPIO1_DR);
        val &= ~(1 << 3);
        writel(val, IMX6U_GPIO1_DR);
    }
    else if(state == LEDOFF)
    {
        val = readl(IMX6U_GPIO1_DR);
        val |= (1 << 3);
        writel(val, IMX6U_GPIO1_DR);
    }
}

int led_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}

static ssize_t led_writer(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    unsigned char databuf[1];
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
    int i =0;
    int ressize[5];
    u32 val;
    struct resource *ledres[5];

    printk("led device and driver has matched!\n");

    for(i=0;i<5;i++)
    {
        ledres[i] = platform_get_resource(pdev, IORESOURCE_MEM, i);
        
        if(!ledres[i])
        {
            dev_err(&pdev->dev, "failed to get resource %d\n", i);
            return -ENXIO;
        }
        ressize[i] = resource_size(ledres[i]);
    }

    IMX6U_CCM_CCGR1 = ioremap(ledres[0]->start, ressize[0]);
    IMX6U_MUX_GPIO1_IO03 = ioremap(ledres[1]->start, ressize[1]);
    IMX6U_PAD_GPIO1_IO03 = ioremap(ledres[2]->start, ressize[2]);
    IMX6U_GPIO1_DR = ioremap(ledres[3]->start, ressize[3]);
    IMX6U_GPIO1_GDIR = ioremap(ledres[4]->start, ressize[4]);

    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3<<26);
    val |= (3<<26);
    writel(val, IMX6U_CCM_CCGR1);

    writel(5, IMX6U_MUX_GPIO1_IO03);

    writel(0x10B0, IMX6U_PAD_GPIO1_IO03);

    val = readl(IMX6U_GPIO1_GDIR);
    val &= ~(1<<3);
    val |= (1<<3);
    writel(val, IMX6U_GPIO1_GDIR);

    val = readl(IMX6U_GPIO1_DR);
    val |=(1<<3);
    writel(val, IMX6U_GPIO1_DR);

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

    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(IMX6U_MUX_GPIO1_IO03);
    iounmap(IMX6U_PAD_GPIO1_IO03);
    iounmap(IMX6U_GPIO1_GDIR);
    iounmap(IMX6U_GPIO1_DR);    
    
    device_destroy(leddev.class, leddev.devid);
    class_destroy(leddev.class);
    cdev_del(&leddev.cdev);
    unregister_chrdev_region(leddev.devid, LEDDEV_CNT);

    return 0;
}

static struct platform_driver led_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "led",
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