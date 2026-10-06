<!-- Source: Linux I2C驱动（9.5）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux I2C驱动（9.5）

可以，按**学习顺序**给你整理成一版完整笔记。
**AP3216C 学习整理**
1. 设备结构体 `ap3216c_dev`  
用来统一保存驱动需要的东西，比如 `devid`、`cdev`、`class`、`device`、`major`、`private_data`，以及 `ir/als/ps` 这三个传感器数据。  
作用：把设备号、字符设备、用户节点、I2C 句柄和传感器数据集中管理。
2. `ap3216c_read_regs()`  
底层多字节读寄存器函数。  
原型一般是：
```c
ap3216c_read_regs(dev, reg, val, len)
```
含义：
- `dev`：设备结构体
- `reg`：起始寄存器地址
- `val`：读数据的缓冲区
- `len`：读取长度
3. `msg[0]` 和 `msg[1]`  
`msg[0]` 先发送寄存器地址，`msg[1]` 再读取数据。  
读寄存器必须“先写地址，再读值”。
4. `i2c_transfer(client->adapter, msg, 2)`  
`adapter` 是 I2C 总线/控制器。  
`msg` 是消息数组。  
`2` 表示这次有两个消息。  
返回值 `ret` 表示实际完成了几个消息：
- `2`：成功
- `<0`：失败
- `!=2`：通常也算失败
5. `flags` 的含义  
- `flags = 0`：写
- `flags = I2C_M_RD`：读  
`0` 是默认写，读要显式标记。
6. 为什么写入要单独定义 `buf`  
因为底层函数要的是**缓冲区地址**，不是一个值。  
所以常写成：
```c
u8 buf = data;
ap3216c_write_regs(dev, reg, &buf, 1);
```
7. `ap3216c_read_reg()` / `ap3216c_write_reg()`  
这是单寄存器读写的封装。  
本质上还是调用 `read_regs()` / `write_regs()`，只是一次只操作 1 个寄存器。
8. `ap3216c_readdata()`  
这是业务数据读取函数。  
它先读出 6 个原始寄存器，再把数据拼成：
- `ir`
- `als`
- `ps`
9. 位移和按位与的原因  
AP3216C 的数据不是整齐对齐放在一个寄存器里，而是分散在多个位域里，所以要：
- `<<`：把高位移到正确位置
- `&`：取出有效位
- `|`：把高低位拼起来
10. `open()` 的作用  
`open()` 里通常会：
- `filp->private_data = &ap3216cdev;`
- 初始化 AP3216C
- 复位传感器
作用：把设备对象挂到文件对象上，后面 `read()` 可以直接拿到。
11. 复位流程  
```c
ap3216c_write_reg(dev, AP3216C_SYSTEMCONG, 0x04);
mdelay(50);
ap3216c_write_reg(dev, AP3216C_SYSTEMCONG, 0x03);
```
意思是：
- `0x04`：复位
- 延时：等待稳定
- `0x03`：退出复位，进入正常工作
12. `mdelay(50)`  
表示延时 50ms，不是 50 秒。  
教程注释写“最少 10ms”，写 50ms 只是更保守，没问题。
13. `ap3216c_read()`  
这是字符设备的 `read` 回调，面向用户空间。  
用户执行 `read(fd, ...)`，最终会进入这里。  
作用：把 `ir/als/ps` 拷贝给用户程序。
14. `err` 的问题  
教程里 `err = copy_to_user(...)`，但最后直接 `return 0`，所以 `err` 没真正用上。  
更规范的写法应该判断失败并返回 `-EFAULT`。
15. `probe()` 的作用  
I2C 设备和驱动匹配成功后执行。  
这里一般做：
- 分配/注册字符设备
- 创建设备节点
- 初始化 `private_data`
- 准备后续读写接口
16. `remove()` 的作用  
驱动卸载时执行。  
这里一般做：
- `cdev_del()`
- `unregister_chrdev_region()`
- `device_destroy()`
- `class_destroy()`
17. 两种匹配方式  
- `i2c_device_id`：匹配设备名
- `of_device_id`：匹配设备树 `compatible`  
两者都能让驱动进入 `probe()`。
18. 匹配设备名是找哪里  
找的是 I2C 设备对象的名字，常见来源是 `i2c_board_info.type`，不是 `/dev` 节点名。
19. `ap3216cApp.c` 用户程序  
流程是：
- `open("/dev/ap3216c")`
- 循环 `read(fd, ...)`
- 打印 `ir/als/ps`
20. 这一章的核心理解  
AP3216C 驱动就是：
```text
I2C 寄存器读写
-> 原始数据解析
-> 字符设备接口
-> 用户空间读取
```
