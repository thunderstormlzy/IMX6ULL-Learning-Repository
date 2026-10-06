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

#define DTSLED_CNT 1
#define DTSLED_NAME "dtsled"
#define LED_ON 1
#define LED_OFF 0

static void __iomem *CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

struct dtsled_dev
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *dev;
    int major;
    int minor;
    struct device_node *nd;
}dtsled;

void led_switch(u8 state)
{
    u32 val = 0;

    if (state == LED_ON) {
        val = readl(GPIO1_DR);
        val &= ~(1 << 3);
        writel(val, GPIO1_DR);
    } else if (state == LED_OFF) {
        val = readl(GPIO1_DR);
        val |= (1 << 3);
        writel(val, GPIO1_DR);
    }
}

static int led_open(struct inode *inode, struct file *file)
{
    file->private_data = &dtsled;
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
    val = copy_from_user(ledbuf, buf, size);
    if (val < 0) {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }
    ledstate = ledbuf[0];

    if(ledbuf[0] == LED_ON) {
        led_switch(LED_ON);
    } else if(ledbuf[0] == LED_OFF) {
        led_switch(LED_OFF);
    } else {
        printk("Invalid LED state!\r\n");
        return -EINVAL;
    }
    return 0;
}

static int led_release(struct inode *inode, struct file *file)
{
    return 0;
}

struct file_operations dtsled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

static int __init led_init(void)
{
    u32 val=0;
    int ret = 0;
    u32 regadta[14];
    const char *str;
    const struct property *proper;


    dtsled.nd = of_find_node_by_path("/led");
    if(dtsled.nd == NULL) {
        printk("led node not find!\r\n");
        return -EINVAL;
    }
    else {
        printk("led node find!\r\n");
    }

    proper = of_get_property(dtsled.nd, "compatible", NULL);
    if(proper == NULL) {
        printk("led compatible property not find!\r\n");
        return -EINVAL;
    }
    else {
        printk("led compatible property find!\r\n");
    }

    ret = of_property_read_string(dtsled.nd, "status", &str);
    if(ret < 0) {
        printk("led status property not find!\r\n");
        return -EINVAL;
    }
    else {
        printk("led status property find!\r\n");
    }

    ret = of_property_read_u32_array(dtsled.nd, "reg", regadta, 10);
    if(ret < 0) {
        printk("led reg property not find!\r\n");
        return -EINVAL;
    }
    else {
        u8 i;
        for(i=0; i<10; i++) {
            printk("regadta[%d] = 0x%x\r\n", i, regadta[i]);
        }
    }

#if 0
    CCM_CCGR1 = ioremap(regadta[0], regadta[1]);
    SW_MUX_GPIO1_IO03 = ioremap(regadta[2], regadta[3]);
    SW_PAD_GPIO1_IO03 = ioremap(regadta[4], regadta[5]);
    GPIO1_DR = ioremap(regadta[6], regadta[7]);
    GPIO1_GDIR = ioremap(regadta[8], regadta[9]);
#else
    CCM_CCGR1 = of_iomap(dtsled.nd, 0);
    SW_MUX_GPIO1_IO03 = of_iomap(dtsled.nd, 1);
    SW_PAD_GPIO1_IO03 = of_iomap(dtsled.nd, 2);
    GPIO1_DR = of_iomap(dtsled.nd, 3);
    GPIO1_GDIR = of_iomap(dtsled.nd, 4);
#endif

    val = readl(CCM_CCGR1);
    val &= ~(3 << 26);
    val|=(3<<26);
    writel(val, CCM_CCGR1);
 
    writel(5, SW_MUX_GPIO1_IO03);

    writel(0x10B0, SW_PAD_GPIO1_IO03);

    val = readl(GPIO1_GDIR);
    val &= ~(1 << 3);
    val |= (1 << 3);
    writel(val, GPIO1_GDIR);

    val = readl(GPIO1_DR);
    val |= (1 << 3);
    writel(val, GPIO1_DR);

if(dtsled.major) 
    {
        dtsled.devid = MKDEV(dtsled.major, 0);
        register_chrdev_region(dtsled.devid, DTSLED_CNT, DTSLED_NAME);
    } 
    else 
    {
        alloc_chrdev_region(&dtsled.devid, 0, DTSLED_CNT, DTSLED_NAME);
        dtsled.major = MAJOR(dtsled.devid);
        dtsled.minor = MINOR(dtsled.devid);
    }
    printk("dtsled major=%d, minor=%d\r\n", dtsled.major, dtsled.minor);

    dtsled.cdev.owner = THIS_MODULE;
    cdev_init(&dtsled.cdev, &dtsled_fops);

    cdev_add(&dtsled.cdev, dtsled.devid, DTSLED_CNT);

    dtsled.class = class_create(THIS_MODULE, DTSLED_NAME);
    if (IS_ERR(dtsled.class)) {
        printk("dtsled class_create failed!\r\n");
        return -1;
    }

    dtsled.dev = device_create(dtsled.class, NULL, dtsled.devid, NULL, DTSLED_NAME);
    if (IS_ERR(dtsled.dev)) {
        printk("dtsled device_create failed!\r\n");
        return -1;
    }

    return 0;
}

static void __exit led_exit(void)
{
    iounmap(CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);    

    cdev_del(&dtsled.cdev);
    unregister_chrdev_region(dtsled.devid, DTSLED_CNT);

    device_destroy(dtsled.class, dtsled.devid);
    class_destroy(dtsled.class);

}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lzy");