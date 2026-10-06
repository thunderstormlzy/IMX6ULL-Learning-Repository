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

#define KEY_CNT 1
#define KEY_NAME "key"

#define KEY_DOWN 0
#define KEY_UP 1
#define KEY1VALUE 1
#define KEY0VALUE 0

struct key_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;
    int key_gpio;
    atomic_t keyvalue;
}keydev;

static int key_open(struct inode *inode, struct file *file)
{
    file->private_data = &keydev;
    return 0;
}

static int keyio_init(void)
{
    int ret;
    keydev.nd = of_find_node_by_path("/key");
    if (keydev.nd == NULL) {
        printk("key node not found!\r\n");
        return -EINVAL;
    } else {
        keydev.key_gpio = of_get_named_gpio(keydev.nd, "key_gpio", 0);
        if (keydev.key_gpio < 0) {    
            printk("key gpio not found!\r\n");
            return -EINVAL;
        }
    }
    printk("key gpio: %d\r\n", keydev.key_gpio);

    ret=gpio_request(keydev.key_gpio, "key");
    if (ret) {
        printk("key gpio request failed!\r\n");
        return ret;
    }
    gpio_direction_input(keydev.key_gpio);

    return 0;
}

static int key_read(struct file *file, char __user *buf, size_t count, loff_t *offt)
{
    int ret;
    unsigned char val;
    struct key_dev *dev = file->private_data;
    if(gpio_get_value(dev->key_gpio) == KEY_DOWN) {
        while(gpio_get_value(dev->key_gpio) == KEY_UP);
        atomic_set(&dev->keyvalue, KEY1VALUE);
    }
    else {
        atomic_set(&dev->keyvalue, KEY0VALUE);
    }

    val = atomic_read(&dev->keyvalue);
    ret = copy_to_user(buf, &val, sizeof(val));
    return ret;
}

static struct file_operations key_fops = {
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
};

static int __init mykey_init(void)
{
    atomic_set(&keydev.keyvalue, KEY0VALUE);
    keyio_init();
    if(keydev.major) {
        keydev.devid = MKDEV(keydev.major, 0);
        register_chrdev_region(keydev.devid, KEY_CNT, KEY_NAME);
    } else {
        alloc_chrdev_region(&keydev.devid, 0, KEY_CNT, KEY_NAME);
        keydev.major = MAJOR(keydev.devid);
        keydev.minor = MINOR(keydev.devid);
    }

    keydev.cdev.owner = THIS_MODULE;
    cdev_init(&keydev.cdev, &key_fops);
    cdev_add(&keydev.cdev, keydev.devid, KEY_CNT);

    keydev.class = class_create(THIS_MODULE, KEY_NAME);
    if (IS_ERR(keydev.class)) {
        return -ENODEV;
    }

    keydev.device = device_create(keydev.class, NULL, keydev.devid, NULL, KEY_NAME);
    if (IS_ERR(keydev.device)) {
        return -ENODEV;
        }
    return 0;
}

static void __exit mykey_exit(void)
{
    device_destroy(keydev.class, keydev.devid);
    class_destroy(keydev.class);
    cdev_del(&keydev.cdev);
    unregister_chrdev_region(keydev.devid, KEY_CNT);
}

module_init(mykey_init);
module_exit(mykey_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lzy");