<!-- Source: Linux下GPIO输入驱动（8.12）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux下GPIO输入驱动（8.12）

GPIO 输入驱动的核心是从设备树获取 GPIO，将其申请并设置为输入，然后在 read 函数中读取 GPIO 电平并返回给应用层。

设备树描述按键 GPIO → 驱动加载时读取设备树 → 申请 GPIO → 设置 GPIO 为输入 → 应用程序 read 设备节点 → 驱动读取 GPIO 电平 → 返回按键值给应用层

输入驱动与输出驱动的差别
LED 输出驱动：
设备树描述 LED GPIO
-> 驱动获取 GPIO
-> gpio_direction_output 设置为输出
-> write 函数接收用户数据
-> gpio_set_value 改变输出电平
GPIO 输入驱动：
设备树描述按键 GPIO
-> 驱动获取 GPIO
-> gpio_direction_input 设置为输入
-> read 函数读取 GPIO 电平
-> gpio_get_value 获取输入状态
-> copy_to_user 返回给应用层
