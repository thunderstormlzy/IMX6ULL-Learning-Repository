<!-- Source: Linux MISC驱动实验（8.24）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux MISC驱动实验（8.24）

int misc_register(struct miscdevice * misc)
misc：要注册的 MISC 设备。
返回值：负数，失败；0，成功。
该命令包含下述内容：
alloc_chrdev_region(); /* 申请设备号 */
cdev_init(); /* 初始化 cdev */
cdev_add(); /* 添加 cdev */
class_create(); /* 创建类 */
device_create(); /* 创建设备 */
int misc_deregister(struct miscdevice *misc)
misc：要注销的 MISC 设备。
返回值：负数，失败；0，成功。
该命令包含下述内容：
cdev_del(); /* 删除 cdev */
unregister_chrdev_region(); /* 注销设备号 */
device_destroy(); /* 删除设备 */
class_destroy(); /* 删除类 */
