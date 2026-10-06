<!-- Source: Linux内核定时器（8.13）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux内核定时器（8.13）

Linux 内核定时器用于在未来某个时间点执行指定回调函数，适合做周期性任务、超时检测、软件闪烁、按键消抖等。它不依赖应用程序循环等待，而是由内核定时机制在超时后调用回调函数。
主要流程
定义 timer_list 定时器 → 初始化定时器并绑定回调函数 → 设置超时时间并启动定时器 → 定时器超时后执行回调函数 → 如果需要周期执行，在回调函数里重新启动定时器 → 驱动卸载时删除定时器

Linux 内核定时器通过 timer_list 描述一个定时任务，mod_timer 设置其到期时间，到期后内核在软中断上下文执行回调函数。若要周期执行，需要在回调函数中重新启动定时器；驱动卸载前必须使用 del_timer_sync 删除定时器，防止模块卸载后回调继续访问无效代码。
回调函数主要内容
void timer_function(unsigned long data)
{
    struct timer_dev *dev = (struct timer_dev *)data;
    unsigned long flags;
    int timerperiod;
    
    //回调具体操作
    
    spin_lock_irqsave(&dev->lock, flags);
    timerperiod = dev->timerperiod;
    spin_unlock_irqrestore(&dev->lock, flags);
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->timerperiod));
    //重启定时器
}
ioctl操作函数主要内容
switch (cmd) {
case OPEN_CMD:
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->timeperiod));
    break;
case CLOSE_CMD:
    del_timer_sync(&dev->timer);
    break;
case SETPERIOD_CMD:
    dev->timeperiod = arg;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->timeperiod));
    break;
}

1. jiffies 是 Linux 内核里的系统节拍计数
2. msecs_to_jiffies() 用于把毫秒转换成 jiffies
3. mod_timer() 的参数是到期时间，不是延时时间，mod_timer 可以启动一个未激活的定时器，也可以修改一个已经激活的定时器的到期时间。
4. 定时器回调默认只执行一次，周期执行要在回调中重新启动
5. 驱动卸载前必须删除定时器，否则模块卸载后定时器回调可能继续执行，访问已经释放的代码或数据，导致内核崩溃。
6. 定时器回调运行在软中断上下文，不能执行可能导致睡眠的操作，例如 copy_to_user、mutex_lock、msleep 等。
