#include <linux/module.h>
#include <linux/ratelimit.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/input/mt.h>
#include <linux/input/touchscreen.h>
#include <linux/input/edt-ft5x06.h>
#include <linux/i2c.h>


#define MAX_SUPPORT_POINTS		5			/* 5点触摸 	*/
#define TOUCH_EVENT_DOWN		0x00		/* 按下 	*/
#define TOUCH_EVENT_UP			0x01		/* 抬起 	*/
#define TOUCH_EVENT_ON			0x02		/* 接触 	*/
#define TOUCH_EVENT_RESERVED	0x03		/* 保留 	*/

#define FT5X06_TD_STATUS_REG	0X02		/*	状态寄存器地址 		*/
#define FT5x06_DEVICE_MODE_REG	0X00 		/* 模式寄存器 			*/
#define FT5426_IDG_MODE_REG		0XA4		/* 中断模式				*/
#define FT5X06_READLEN			29			/* 要读取的寄存器个数 	*/


struct ft5x06_dev{
    struct device_node	*nd; 
    int irq_pin,reset_pin;
    int irqnum;
    void *private_data;
    struct input_dev *input;
    struct i2c_client *client;
}ft5x06;


static int ft5x06_ts_reset(struct i2c_client *client, struct ft5x06_dev *dev)
{
    int ret = 0;

    if (gpio_is_valid(dev->reset_pin)) {  	
        ret = devm_gpio_request_one(&client->dev,	
                    dev->reset_pin, GPIOF_OUT_INIT_LOW,
                    "edt-ft5x06 reset");
        if (ret) {
            return ret;
        }

        msleep(5);
        gpio_set_value(dev->reset_pin, 1);	/* 输出高电平，停止复位 */
        msleep(300);
    }

    return 0;
}


static int ft5x06_read_regs(struct ft5x06_dev *dev, u8 reg, void *val, int len)
{
    int ret;
    struct i2c_msg msg[2];
    struct i2c_client *client = (struct i2c_client *)dev->client;
    
    msg[0].addr = client->addr;	
    msg[0].flags = 0;		
    msg[0].buf = &reg;		
    msg[0].len = 1;		
    
    msg[1].addr = client->addr;	
    msg[1].flags = I2C_M_RD;	
    msg[1].buf = val;		
    msg[1].len = len;		
    
    ret = i2c_transfer(client->adapter, msg, 2);
    if(ret == 2) {
        ret = 0;
    } else {
        ret = -EREMOTEIO;
    }

    return ret;
}


static s32 ft5x06_write_regs(struct ft5x06_dev *dev, u8 reg, u8 *buf, u8 len)
{
    u8 b[256];
    struct i2c_msg msg;
    struct i2c_client *client = (struct i2c_client *)dev->client;
    
    b[0] = reg;					
    memcpy(&b[1],buf,len);		
        
    msg.addr = client->addr;	
    msg.flags = 0;				
    msg.buf = b;				
    msg.len = len + 1;			
    
    return i2c_transfer(client->adapter, &msg, 1);
}


static void ft5x06_write_reg(struct ft5x06_dev *dev, u8 reg, u8 data)
{
    u8 buf = 0;
    buf = data;

    ft5x06_write_regs(dev, reg, &buf, 1);
}


static irqreturn_t ft5x06_handler(int irq, void *dev_id)
{
    struct ft5x06_dev *multidata = dev_id;
    u8 rdbuf[29];
    int i, type, x, y, id;
    int offset, tplen;
    int ret;
    bool down;

    offset =1;
    tplen = 6;

    memset(rdbuf, 0, sizeof(rdbuf));		

    ret = ft5x06_read_regs(multidata, FT5X06_TD_STATUS_REG, rdbuf, FT5X06_READLEN);
    if (ret) {
        goto fail;
    }

    for(i=0;i<MAX_SUPPORT_POINTS;i++){
        u8 *buf = &rdbuf[i * tplen + offset];

        type = buf[0] >> 6;     
        if (type == TOUCH_EVENT_RESERVED)
            continue;

        x = ((buf[2] << 8) | buf[3]) & 0x0fff;
        y = ((buf[0] << 8) | buf[1]) & 0x0fff;
        
        id = (buf[2] >> 4) & 0x0f;
        down = type != TOUCH_EVENT_UP;

        input_mt_slot(multidata->input, id);
        input_mt_report_slot_state(multidata->input, MT_TOOL_FINGER, down);
        
        if (!down)
            continue;

        input_report_abs(multidata->input, ABS_MT_POSITION_X, x);
        input_report_abs(multidata->input, ABS_MT_POSITION_Y, y);
    }

    input_mt_report_pointer_emulation(multidata->input, true);
    input_sync(multidata->input);

    fail:
    return IRQ_HANDLED;

}


static int ft5x06_ts_irq(struct i2c_client *client, struct ft5x06_dev *dev)
{
    int ret = 0;

    if(gpio_is_valid(dev->irq_pin))
    {
        ret = devm_gpio_request_one(&client->dev, dev->irq_pin,
                    GPIOF_IN, "ft5x06 irq");
        if (ret) {
            dev_err(&client->dev,
                "Failed to request GPIO %d, error %d\n",
                dev->irq_pin, ret);
            return ret;
        }
    }


    ret = devm_request_threaded_irq(&client->dev, client->irq, NULL,
                    ft5x06_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
                    client->name, &ft5x06);

    if (ret) {
        dev_err(&client->dev, "Unable to request touchscreen IRQ.\n");
        return ret;
    }

    return 0;
}


static int ft5x06_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;

    ft5x06.client = client;

    ft5x06.irq_pin = of_get_named_gpio(client->dev.of_node, "interrupt-gpios", 0);
    ft5x06.reset_pin = of_get_named_gpio(client->dev.of_node, "reset-gpios", 0);

    ret = ft5x06_ts_reset(client, &ft5x06);
    if(ret < 0) {
        goto fail;
    }

    ret = ft5x06_ts_irq(client, &ft5x06);
    if(ret < 0) {
        goto fail;
    }

    ft5x06_write_reg(&ft5x06, FT5x06_DEVICE_MODE_REG, 0); 	
    ft5x06_write_reg(&ft5x06, FT5426_IDG_MODE_REG, 1);
    
    ft5x06.input = devm_input_allocate_device(&client->dev);
    if (!ft5x06.input) {
        ret = -ENOMEM;
        goto fail;
    }

    ft5x06.input->name = client->name;
    ft5x06.input->id.bustype = BUS_I2C;
    ft5x06.input->dev.parent = &client->dev;

    __set_bit(EV_KEY, ft5x06.input->evbit);
    __set_bit(EV_ABS, ft5x06.input->evbit);
    __set_bit(BTN_TOUCH, ft5x06.input->keybit);

    input_set_abs_params(ft5x06.input, ABS_X, 0, 1024, 0, 0);
	input_set_abs_params(ft5x06.input, ABS_Y, 0, 600, 0, 0);
    input_set_abs_params(ft5x06.input, ABS_MT_POSITION_X,0, 1024, 0, 0);
	input_set_abs_params(ft5x06.input, ABS_MT_POSITION_Y,0, 600, 0, 0);	
    ret=input_mt_init_slots(ft5x06.input, MAX_SUPPORT_POINTS, 0);

    if (ret) {
        goto fail;
    }

    ret = input_register_device(ft5x06.input);
    if (ret)
        goto fail;

    return 0;

    fail:
    return ret;
}


static int ft5x06_ts_remove(struct i2c_client *client)
{
    input_unregister_device(ft5x06.input);
    return 0;
}


static const struct i2c_device_id ft5x06_ts_id[] = {
    { "edt-ft5206", 0, },
    { "edt-ft5426", 0, },
    { /* sentinel */ }
};


static const struct of_device_id ft5x06_of_match[] = {
    { .compatible = "edt,edt-ft5206", },
    { .compatible = "edt,edt-ft5426", },
    { /* sentinel */ }
};


static struct i2c_driver ft5x06_ts_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "edt_ft5x06",
        .of_match_table = of_match_ptr(ft5x06_of_match),
    },
    .id_table = ft5x06_ts_id,
    .probe    = ft5x06_ts_probe,
    .remove   = ft5x06_ts_remove,
};


static int __init ft5x06_init(void)
{
    int ret = 0;

    ret = i2c_add_driver(&ft5x06_ts_driver);

    return ret;
}


static void __exit ft5x06_exit(void)
{
    i2c_del_driver(&ft5x06_ts_driver);
}


module_init(ft5x06_init);
module_exit(ft5x06_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lzy");