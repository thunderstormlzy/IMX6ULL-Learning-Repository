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
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include "icm20608reg.h"


#define ICM20608_CNT 1
#define ICM20608_NAME "icm20608"


struct icm20608_dev {
    dev_t devid;                /* 设备号 */
    struct cdev cdev;          /* cdev */
    struct class *class;       /* 类 */
    struct device *device;     /* 设备 */
    struct device_node *nd;    /* 设备节点 */
    int major;                 /* 主设备号 */
    void *private_data;        /* 私有数据 */
    signed int gyro_x_adc;      /* 陀螺仪X轴原始值 */
    signed int gyro_y_adc;      /* 陀螺仪Y轴原始值 */
    signed int gyro_z_adc;      /* 陀螺仪Z轴原始值 */
    signed int accel_x_adc;     /* 加速度X轴原始值 */
    signed int accel_y_adc;     /* 加速度Y轴原始值 */
    signed int accel_z_adc;     /* 加速度Z轴原始值 */
    signed int temp_adc;        /* 温度原始值 */
}icm20608dev;


static int icm20608_read_regs(struct icm20608_dev *dev, u8 reg, u8 *buf, int len)
{
    int ret =-1;
    unsigned char txdata[1];
    unsigned char *rxdata;
    struct spi_message msg;
    struct spi_transfer *t;
    struct spi_device *spi = (struct spi_device *)dev->private_data;

    t=kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    if(!t){
        return -ENOMEM;
    }

    rxdata=kzalloc(sizeof(unsigned char)*len, GFP_KERNEL);
    if(!rxdata){
        goto out1;
    }

    txdata[0]=reg|0x80; /* 读寄存器，最高位为1 */
    t->tx_buf=txdata;
    t->rx_buf=rxdata;
    t->len=len+1;

    spi_message_init(&msg);
    spi_message_add_tail(t, &msg);
    ret=spi_sync(spi, &msg);
    if(ret){
        goto out2;
    }

    memcpy(buf, rxdata+1, len);

    out2:
    kfree(rxdata);
    out1:
    kfree(t);

    return ret;
}


static s32 icm20608_write_regs(struct icm20608_dev *dev, u8 reg, u8*buf, int len)
{
    int ret =-1;
    unsigned char *txdata;
    struct spi_message msg;
    struct spi_transfer *t;
    struct spi_device *spi = (struct spi_device *)dev->private_data;

    t=kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    if(!t){
        return -ENOMEM;
    }

    txdata=kzalloc(len+1, GFP_KERNEL);
    if(!txdata){
        goto out1;
    }

    txdata[0]=reg&0x7f; /* 写寄存器，最高位为0 */
    memcpy(txdata+1, buf, len);

    t->tx_buf=txdata;
    t->len=len+1;

    spi_message_init(&msg);
    spi_message_add_tail(t, &msg);
    ret=spi_sync(spi, &msg);
    if(ret<0){
        goto out2;
    }

    out2:
    kfree(txdata);
    out1:
    kfree(t);

    return ret;
}


static unsigned char icm20608_read_onereg(struct icm20608_dev *dev, u8 reg)
{
    u8 data=0;
    icm20608_read_regs(dev, reg, &data, 1);
    return data;
}


static void icm20608_write_onereg(struct icm20608_dev *dev, u8 reg, u8 data)
{
    icm20608_write_regs(dev, reg, &data, 1);
}


void icm20608_readdata(struct icm20608_dev *dev)
{
    u8 data[14]={0};
    icm20608_read_regs(dev, ICM20_ACCEL_XOUT_H, data, 14);

    dev->accel_x_adc=(s16)((data[0]<<8)|data[1]);
    dev->accel_y_adc=(s16)((data[2]<<8)|data[3]);
    dev->accel_z_adc=(s16)((data[4]<<8)|data[5]);
    dev->temp_adc=(s16)((data[6]<<8)|data[7]);
    dev->gyro_x_adc=(s16)((data[8]<<8)|data[9]);
    dev->gyro_y_adc=(s16)((data[10]<<8)|data[11]);
    dev->gyro_z_adc=(s16)((data[12]<<8)|data[13]);
}


static int icm20608_open(struct inode *inode, struct file *file)
{
    file->private_data=&icm20608dev;
    return 0;
}


static ssize_t icm20608_read(struct file *file, char __user *buf, size_t cnt, loff_t *f_pos)
{
    signed int data[7];
    long err=0;
    struct icm20608_dev *dev = (struct icm20608_dev *)file->private_data;

    icm20608_readdata(dev);
    data[0]=dev->accel_x_adc;
    data[1]=dev->accel_y_adc;
    data[2]=dev->accel_z_adc;
    data[3]=dev->temp_adc;
    data[4]=dev->gyro_x_adc;
    data[5]=dev->gyro_y_adc;
    data[6]=dev->gyro_z_adc;

    err=copy_to_user(buf, data, sizeof(data));
    if(err){
        return -EFAULT;
    }

    return sizeof(data);
}


static int icm20608_release(struct inode *inode, struct file *file)
{
    return 0;
}


static const struct file_operations icm20608_fops = {   
    .owner = THIS_MODULE,
    .open = icm20608_open,
    .read = icm20608_read,
    .release = icm20608_release,
};


void icm20608_reginit(void)
{
	u8 value = 0;
	
	icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_1, 0x80);
	mdelay(50);
	icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_1, 0x01);
	mdelay(50);

	value = icm20608_read_onereg(&icm20608dev, ICM20_WHO_AM_I);
	printk("ICM20608 ID = %#X\r\n", value);	

	icm20608_write_onereg(&icm20608dev, ICM20_SMPLRT_DIV, 0x00); 	/* 输出速率是内部采样率					*/
	icm20608_write_onereg(&icm20608dev, ICM20_GYRO_CONFIG, 0x18); 	/* 陀螺仪±2000dps量程 				*/
	icm20608_write_onereg(&icm20608dev, ICM20_ACCEL_CONFIG, 0x18); 	/* 加速度计±16G量程 					*/
	icm20608_write_onereg(&icm20608dev, ICM20_CONFIG, 0x04); 		/* 陀螺仪低通滤波BW=20Hz 				*/
	icm20608_write_onereg(&icm20608dev, ICM20_ACCEL_CONFIG2, 0x04); /* 加速度计低通滤波BW=21.2Hz 			*/
	icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_2, 0x00); 	/* 打开加速度计和陀螺仪所有轴 				*/
	icm20608_write_onereg(&icm20608dev, ICM20_LP_MODE_CFG, 0x00); 	/* 关闭低功耗 						*/
	icm20608_write_onereg(&icm20608dev, ICM20_FIFO_EN, 0x00);		/* 关闭FIFO						*/
}


static int icm20608_probe(struct spi_device *spi)
{
    if(icm20608dev.major){
        icm20608dev.devid=MKDEV(icm20608dev.major, 0);
        register_chrdev_region(icm20608dev.devid, ICM20608_CNT, ICM20608_NAME);
    }else{
        alloc_chrdev_region(&icm20608dev.devid, 0, ICM20608_CNT, ICM20608_NAME);
        icm20608dev.major=MAJOR(icm20608dev.devid);
    }

    cdev_init(&icm20608dev.cdev, &icm20608_fops);
    cdev_add(&icm20608dev.cdev, icm20608dev.devid, ICM20608_CNT);

    icm20608dev.class=class_create(THIS_MODULE, ICM20608_NAME);
    if(IS_ERR(icm20608dev.class)){
        return PTR_ERR(icm20608dev.class);
    }

    icm20608dev.device=device_create(icm20608dev.class, NULL, icm20608dev.devid, NULL, ICM20608_NAME);
    if(IS_ERR(icm20608dev.device)){
        return PTR_ERR(icm20608dev.device);
    }

    spi->mode=SPI_MODE_0;
    spi_setup(spi);
    icm20608dev.private_data=spi;

    icm20608_reginit();
    return 0;
}


static int icm20608_remove(struct spi_device *spi)
{
    cdev_del(&icm20608dev.cdev);
    unregister_chrdev_region(icm20608dev.devid, ICM20608_CNT);
    device_destroy(icm20608dev.class, icm20608dev.devid);
    class_destroy(icm20608dev.class);

    return 0;
}


static const struct spi_device_id icm20608_id[] = {
    {"icm20608", 0},
    {}
};


static const struct of_device_id icm20608_of_match[] = {
    {.compatible = "icm20608"},
    {},
};


static struct spi_driver icm20608_driver = {
    .driver = {
        .name = ICM20608_NAME,
        .owner = THIS_MODULE,
        .of_match_table = icm20608_of_match,
    },
    .probe = icm20608_probe,
    .remove = icm20608_remove,
    .id_table = icm20608_id,
};


static int __init icm20608_init(void)
{
    return spi_register_driver(&icm20608_driver);
}


static void __exit icm20608_exit(void)
{
    spi_unregister_driver(&icm20608_driver);
}


module_init(icm20608_init);
module_exit(icm20608_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lzy");