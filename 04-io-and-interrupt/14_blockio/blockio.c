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
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

#define IRQ_CNT 1
#define IRQ_NAME "blockio"
#define KEY0VALUE 0x01
#define INVAKEY 0xFF
#define KEY_NUM 1

struct irq_keydesc {
    int gpio;
    int irqnum;
    unsigned char value;
    char name[10];
    irqreturn_t (*handler)(int, void *);
};

struct irq_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *dev;
    int major;
    int minor;
    struct device_node *nd;
    atomic_t keyvalue;
    atomic_t releasekey;
    struct timer_list timer;
    struct irq_keydesc irqkeydesc[KEY_NUM];
    unsigned char curkeynum;
    wait_queue_head_t wait;
}irqdev;

static irqreturn_t key0_handler(int irq, void *dev_id)
{
    struct irq_dev *dev = (struct irq_dev *)dev_id;

    dev->curkeynum = 0 ;
    dev->timer.data = (volatile long)dev;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(100));
    return IRQ_RETVAL(IRQ_HANDLED);
}

void timer_function(unsigned long data)
{
    unsigned char value;
    unsigned char num;
    struct irq_keydesc *keydesc;
    struct irq_dev *dev = (struct irq_dev *)data;

    num=dev->curkeynum;
    keydesc=&dev->irqkeydesc[num];
    value=gpio_get_value(keydesc->gpio);

    if(value==0)
    {
        atomic_set(&dev->keyvalue, keydesc->value);
    }
    else
    {
        atomic_set(&dev->keyvalue, 0x80|keydesc->value);
        atomic_set(&dev->releasekey, 1);
    }

    if(atomic_read(&dev->releasekey))
    {
        wake_up_interruptible(&dev->wait);
    }
}

static int keyio_init(void)
{
    unsigned int i = 0;
    int ret = 0;

    irqdev.nd = of_find_node_by_path("/key");
    if(irqdev.nd == NULL)
    {
        printk("kcan't find key node\n");
        return -1;
    }

    for(i=0;i<KEY_NUM;i++)
    {
        irqdev.irqkeydesc[i].gpio = of_get_named_gpio(irqdev.nd, "key_gpio", i);
        if(irqdev.irqkeydesc[i].gpio < 0)
        {
            printk("kcan't find key-gpio node\n");
            return -1;
        }
    }

    for(i=0;i<KEY_NUM;i++)
    {
        memset(irqdev.irqkeydesc[i].name, 0, sizeof(irqdev.irqkeydesc[i].name));
        sprintf(irqdev.irqkeydesc[i].name, "key%d", i);
        gpio_request(irqdev.irqkeydesc[i].gpio, irqdev.irqkeydesc[i].name);
        gpio_direction_input(irqdev.irqkeydesc[i].gpio);
        irqdev.irqkeydesc[i].irqnum = irq_of_parse_and_map(irqdev.nd, i);
    }

    irqdev.irqkeydesc[0].handler = key0_handler;
    irqdev.irqkeydesc[0].value = KEY0VALUE;

    for(i=0;i<KEY_NUM;i++)
    {
        ret = request_irq(irqdev.irqkeydesc[i].irqnum, irqdev.irqkeydesc[i].handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, irqdev.irqkeydesc[i].name, &irqdev);
        if(ret < 0)
        {
            printk("irq %d request failed\n", irqdev.irqkeydesc[i].irqnum);
            return -1;
        }
    }

    init_timer(&irqdev.timer);
    irqdev.timer.function = timer_function;

    init_waitqueue_head(&irqdev.wait);
    return 0;
}

static int irq_open(struct inode *inode, struct file *file)
{
    file->private_data = &irqdev;
    return 0;
}

static ssize_t irq_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct irq_dev *dev = file->private_data;
    unsigned char keyvalue=0;
    unsigned char releasekey=0;
    int ret=0;

    DECLARE_WAITQUEUE(wait, current);
    if(atomic_read(&dev->releasekey)==0)
    {
        add_wait_queue(&dev->wait, &wait);
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
        if(signal_pending(current))
        {
            ret = -ERESTARTSYS;
            goto wait_error;
        }
        set_current_state(TASK_RUNNING);
        remove_wait_queue(&dev->wait, &wait);
    }


    keyvalue=atomic_read(&dev->keyvalue);
    releasekey=atomic_read(&dev->releasekey);

    if(releasekey)
    {
        if(keyvalue&0x80)
        {
            keyvalue&=~0x80;
            ret = copy_to_user(buf, &keyvalue, sizeof(keyvalue));
        }
        else
        {
            goto error;
        }


        atomic_set(&dev->releasekey, 0);
    }
    else
    {
        goto error;
    }

    return 0;

    wait_error:
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&dev->wait, &wait);
    return ret;

    error:
    return -EINVAL;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = irq_open,
    .read = irq_read,
};

static int __init irq_init(void)
{
    int ret = 0;

    if(irqdev.major)
    {
        irqdev.devid = MKDEV(irqdev.major, 0);
        register_chrdev_region(irqdev.devid, IRQ_CNT, IRQ_NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&irqdev.devid, 0, IRQ_CNT, IRQ_NAME);
        if(ret < 0)
        {
            printk("alloc_chrdev_region failed\n");
            return -1;
        }
        irqdev.major = MAJOR(irqdev.devid);
        irqdev.minor = MINOR(irqdev.devid);
    }

    cdev_init(&irqdev.cdev, &fops);
    cdev_add(&irqdev.cdev, irqdev.devid, IRQ_CNT);

    irqdev.class = class_create(THIS_MODULE, IRQ_NAME);
    if(IS_ERR(irqdev.class))
    {
        return PTR_ERR(irqdev.class);
    }

    irqdev.dev = device_create(irqdev.class, NULL, irqdev.devid, NULL, IRQ_NAME);
    if(IS_ERR(irqdev.dev))
    {
        return PTR_ERR(irqdev.dev);
    }

    atomic_set(&irqdev.keyvalue, INVAKEY);
    atomic_set(&irqdev.releasekey, 0);
    ret = keyio_init();
    if(ret < 0)
    {
        printk("keyio_init failed\n");
        return -1;
    }

    return 0;
}

static void __exit my_irq_exit(void)
{
    unsigned int i = 0;

    device_destroy(irqdev.class, irqdev.devid);

    for(i=0;i<KEY_NUM;i++)
    {
        free_irq(irqdev.irqkeydesc[i].irqnum, &irqdev);
        gpio_free(irqdev.irqkeydesc[i].gpio);
    }


    class_destroy(irqdev.class);
    cdev_del(&irqdev.cdev);
    unregister_chrdev_region(irqdev.devid, IRQ_CNT);
}

module_init(irq_init);
module_exit(my_irq_exit);

MODULE_LICENSE("GPL");  
MODULE_AUTHOR("lzy");
