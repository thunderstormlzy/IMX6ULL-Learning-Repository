<!-- Source: Linux Input子系统实验（8.27）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux Input子系统实验（8.27）

在编写 input 设备驱动的时候我们需要先申请一个 input_dev 结构体变量，使用 input_allocate_device 函数来申请一个 input_dev
struct input_dev *input_allocate_device(void)
参数：无。
返回值：申请到的 input_dev。
如果要注销的 input 设备的话需要使用 input_free_device 函数来释放掉前面申请到的 input_dev，
void input_free_device(struct input_dev *dev)
dev：需要释放的 input_dev。
返回值：无。
申请好一个 input_dev 以后就需要初始化这个 input_dev，需要初始化的内容主要为事件类型(evbit)和事件值(keybit)这两种。
input_dev 初始化完成以后就需要向 Linux 内核注册 input_dev 了，需要用到 input_register_device 函数
int input_register_device(struct input_dev *dev)
dev：要注册的 input_dev 。
返回值：0，input_dev 注册成功；负值，input_dev 注册失败。
同样的，注销 input 驱动的时候也需要使用 input_unregister_device 函数来注销掉前面注册的 input_dev
void input_unregister_device(struct input_dev *dev)
dev：要注销的 input_dev 。
返回值：无。
综上所述，input的流程为
①、使用 input_allocate_device 函数申请一个 input_dev。
②、初始化 input_dev 的事件类型以及事件值。
③、使用 input_register_device 函数向 Linux 系统注册前面初始化好的 input_dev。
④、卸载 input 驱动的时候需要先使用 input_unregister_device 函数注销掉注册的 input_dev，然后使用 input_free_device 函数释放掉前面申请的 input_dev。
上报输入事件
void input_event(	struct input_dev *dev, 
 				
unsigned int type, 
 				
unsigned int code, 
 				
int value)
dev：需要上报的 input_dev。
type: 上报的事件类型，比如 EV_KEY。
code：事件码，也就是我们注册的按键值，比如 KEY_0、KEY_1 等等。
value：事件值，比如 1 表示按键按下，0 表示按键松开。
返回值：无。
上报事件以后还需要使用 input_sync 函数来告诉 Linux 内核 input 子系统上报结束，input_sync 函数本质是上报一个同步事件
void input_sync(struct input_dev *dev)
函数参数和返回值含义如下：
dev：需要上报同步事件的 input_dev。
返回值：无。
需要把input的
struct input_dev *input_allocate_device(void)
函数和cdev和操作函数的区别列出来；input的注册函数和class/device_create的区别列出来
