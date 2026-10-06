<!-- Source: platform设备驱动实验（8.21）.note | Exported from local Youdao database on 2026-10-06 -->

# platform设备驱动实验（8.21）

platform相比于之前的代码编写更注重分离和分层，分为platform_bus，platform_driver和platform_device三部分。
platform_bus是虚拟总线，负责后两者的匹配，共有四种匹配方法。
if (of_driver_match_device(dev, drv))
return 1;
OF 类型的匹配，也就是设备树采用的匹配方式，of_driver_match_device 函数定义在文件 include/linux/of_device.h 中。device_driver 结构体(表示设备驱动)中有个名为 of_match_table 的成员变量，此成员变量保存着驱动的 compatible 匹配表，设备树中的每个设备节点的 compatible 属性会和 of_match_table 表中的所有成员比较，查看是否有相同的条目，如果有的话就表示设备和此驱动匹配，设备和驱动匹配成功以后 probe 函数就会执行（
一般用于设备树匹配
）。
if (acpi_driver_match_device(dev, drv))
return 1;
ACPI 匹配方式，内核根据固件里的 ACPI 表，拿设备的硬件标识去和驱动的 ACPI ID 表匹配。
if (pdrv->id_table)
return platform_match_id(pdrv->id_table, pdev) != NULL;
id_table 匹配，每个 platform_driver 结构体有一个 id_table成员变量，顾名思义，保存了很多 id 信息。这些 id 信息存放着这个 platformd 驱动所支持的驱动类型。
return (strcmp(pdev->name, drv->name) == 0);
name字段匹配，直接比较驱动和设备的 name 字段，看看是不是相等，如果相等的话就匹配成功（
在无设备树匹配中较为常用
）。
platform_driver是platform驱动，负责描述驱动行为。
int platform_driver_register (struct platform_driver *driver)
driver：要注册的 platform 驱动。
返回值：负数，失败；0，成功
定义并初始化好 platform_driver 结构体变量以后，需要在驱动入口函数里面调用 platform_driver_register 函数向 Linux 内核注册一个 platform 驱动
void platform_driver_unregister(struct platform_driver *drv)
drv：要卸载的 platform 驱动。
返回值：无。
probe 函数
，当驱动和设备匹配成功以后此函数就会执行，以前在驱动入口 init 函数里面编写的字符设备驱动程序就全部放到此 probe 函数里面。比如注册字符设备驱动、添加 cdev、创建类等等。
remove 函数
，platform_driver 结构体中的 remove 成员变量，当关闭 platform 设备驱动的时候此函数就会执行，驱动卸载时释放资源，以前在驱动卸载 exit 函数里面要做的事情就放到此函数中来。比如，使用 iounmap 释放内存、删除 cdev，注销设备号等等。
platform_device是platform设备，负责描述硬件资源。
int platform_device_register(struct platform_device *pdev)
pdev：要注册的 platform 设备。
返回值：负数，失败；0，成功。
void platform_device_unregister(struct platform_device *pdev)
pdev：要注销的 platform 设备。
返回值：无。
