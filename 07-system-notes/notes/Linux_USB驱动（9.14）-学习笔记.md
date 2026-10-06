<!-- Source: Linux USB驱动（9.14）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux USB驱动（9.14）

# 第67章 Linux USB驱动实验重点笔记
## 一、本章定位
第67章不是教你从零写USB驱动，而是讲：
```text
USB基础概念
I.MX6ULL的USB接口
ALPHA开发板USB硬件连接
Linux内核自带USB Host驱动使用
Linux USB Gadget从机功能使用
```
重点是会配置、会测试、能看懂USB设备识别流程。
## 二、USB基础重点
USB是通用串行总线，采用主从结构：
```text
Host：主机，例如电脑、开发板USB Host口
Device：从机，例如U盘、鼠标、键盘、USB声卡
```
USB通信必须是一主一从，不能两个Host直接通信，也不能两个Device直接通信。
USB常见速度：
```text
Low Speed：1.5Mbps
Full Speed：12Mbps
High Speed：480Mbps，USB2.0常见高速模式
```
I.MX6ULL支持USB2.0，因此最高就是480Mbps。
## 三、USB电气信号
USB2.0主要有四根线：
```text
VBUS：5V供电
D-：差分数据负
D+：差分数据正
GND：地
```
Mini USB / Micro USB / Type-C用于OTG时，还会涉及：
```text
ID线：判断当前设备做Host还是Device
```
OTG判断方式：
```text
ID = 0：Host模式
ID = 1：Device模式
```
所以OTG线的本质之一，就是把ID线拉到合适状态，让接口进入Host模式。
## 四、USB拓扑结构
USB是树状结构：
```text
USB Host
   ↓
Root Hub
   ↓
USB Hub
   ↓
USB Device
```
重点理解：
```text
一个USB主控制器最多支持127个USB设备
USB Hub只能扩展接口数量，不能扩展总带宽
```
比如I.MX6ULL的一个USB2.0控制器最大带宽是480Mbps，接了Hub以后，下面所有设备共享这480Mbps。
## 五、I.MX6ULL USB接口
I.MX6ULL内部有两个USB控制器：
```text
USB OTG1
USB OTG2
```
两个都支持USB2.0，也都支持OTG。
在ALPHA开发板上，一般是：
```text
USB OTG1：用于OTG接口，可做Host或Device
USB OTG2：接到USB HUB芯片，扩展出多个USB Host口
```
ALPHA板通过GL850G或SL2.1A这类USB HUB芯片，把一路USB扩展成多路Host接口。
## 六、Host和Device区别
Host模式：
```text
开发板作为主机
外接鼠标、键盘、U盘、USB WiFi、4G模块等
```
Device模式：
```text
开发板作为USB设备
插到电脑上，让电脑识别开发板
```
例子：
```text
开发板读U盘：开发板是Host，U盘是Device
电脑读开发板模拟U盘：电脑是Host，开发板是Device
```
## 七、USB描述符和枚举
USB设备插入后，主机会进行枚举。
枚举可以理解为：
```text
主机发现USB设备
    ↓
复位USB设备
    ↓
读取设备描述符
    ↓
给设备分配地址
    ↓
读取配置、接口、端点等描述符
    ↓
根据描述符加载对应驱动
```
描述符的作用是告诉主机：
```text
我是什么设备
厂商ID和产品ID是什么
我有几个接口
每个接口有什么功能
使用哪些端点通信
需要什么驱动
```
所以Linux能自动识别U盘、鼠标、键盘，本质上就是通过枚举读取描述符后匹配对应驱动。
## 八、USB传输类型
这部分了解即可，但要知道各自用途：
```text
控制传输：用于枚举、配置、读取描述符
批量传输：用于U盘、文件传输，可靠但实时性不强
中断传输：用于鼠标、键盘，数据量小但响应要快
等时传输：用于音频、视频，强调实时性，允许少量丢包
```
对应关系可以这样记：
```text
枚举配置：控制传输
U盘：批量传输
鼠标键盘：中断传输
USB声卡/摄像头：等时传输
```
## 九、USB Host实验重点
Host实验主要测试：
```text
USB鼠标
USB键盘
U盘
```
鼠标键盘属于HID设备，需要关注内核配置：
```text
HID support
Generic HID driver
USB HID transport layer
```
鼠标键盘识别后，一般会生成：
```text
/dev/input/eventX
```
这和前面的input子系统能联系起来。
U盘属于USB Mass Storage设备，需要关注：
```text
SCSI disk support
USB Mass Storage support
```
U盘插入后一般生成：
```text
/dev/sda
/dev/sda1
```
其中：
```text
/dev/sda：整个U盘
/dev/sda1：U盘第一个分区
```
挂载命令示例：
```bash
mkdir /mnt/usb_disk -p
mount /dev/sda1 /mnt/usb_disk -t vfat -o iocharset=utf8
```
拔出前建议：
```bash
sync
umount /mnt/usb_disk
```
`sync`是把缓存数据写入U盘，避免数据还没真正写进去就拔掉。
## 十、USB OTG实验重点
OTG实验分两类：
```text
OTG Host
OTG Device
```
OTG Host：
```text
开发板通过OTG口接U盘、鼠标、键盘
测试方法和普通Host基本一样
```
OTG Device：
```text
开发板插到电脑上
电脑把开发板识别成某种USB设备
```
第67章主要做两个Device实验：
```text
模拟U盘
模拟USB声卡
```
## 十一、设备树重点
本章设备树最重要的是OTG ID引脚。
教程里会修改类似：
```dts
MX6UL_PAD_GPIO1_IO00__ANATOP_OTG1_ID 0x13058
```
作用：
```text
把GPIO1_IO00复用为USB OTG1 ID功能
并设置合适的电气属性，让OTG默认进入正确模式
```
如果ID脚状态不对，可能导致：
```text
OTG口不能正确识别Host/Device模式
插U盘没反应
连接电脑没反应
```
## 十二、USB Gadget是什么
USB Gadget是Linux中让开发板模拟成USB设备的框架。
普通Host驱动是：
```text
开发板识别别人
```
USB Gadget是：
```text
让别人识别开发板
```
例如：
```text
g_mass_storage：让开发板模拟U盘
g_audio：让开发板模拟USB声卡
```
这部分很有价值，因为它体现了Linux USB从机框架。
## 十三、模拟U盘实验流程
大致流程：
```text
配置USB Gadget Mass Storage
    ↓
编译出相关.ko模块
    ↓
把模块拷贝到rootfs
    ↓
开发板通过OTG口连接电脑
    ↓
加载g_mass_storage
    ↓
电脑识别出一个U盘
```
相关模块：
```text
libcomposite.ko
usb_f_mass_storage.ko
g_mass_storage.ko
```
加载时示例：
```bash
modprobe libcomposite
modprobe usb_f_mass_storage
modprobe g_mass_storage file=/dev/sda1 removable=1
```
注意：
```text
file=/dev/sda1表示把开发板上的/dev/sda1作为模拟U盘的存储区域
```
不建议把eMMC或NAND根文件系统分区直接拿来模拟U盘，否则电脑可能提示格式化，甚至造成文件系统损坏。
## 十四、USB声卡实验流程
开发板可以模拟成USB声卡，让电脑把声音通过USB传给开发板播放。
大致流程：
```text
先保证开发板本地声卡能正常播放
    ↓
配置USB Gadget Audio
    ↓
编译并加载g_audio相关模块
    ↓
开发板通过OTG连接电脑
    ↓
电脑识别出USB声卡
    ↓
电脑选择该USB声卡输出
    ↓
开发板播放电脑声音
```
相关模块：
```text
libcomposite.ko
usb_f_uac1.ko
g_audio.ko
```
这个实验和第65章音频驱动有关，前提是WM8960声卡已经能正常工作。
## 十五、本章哪些需要重点看
重点看：
```text
USB Host和Device的区别
USB OTG中ID线的作用
I.MX6ULL两个USB控制器的关系
ALPHA板USB HUB和OTG硬件连接
USB枚举流程
HID设备和input子系统的关系
U盘对应的/dev/sda、/dev/sda1
USB Gadget框架
g_mass_storage和g_audio实验流程
```
了解即可：
```text
USB1.0/2.0/3.0/4.0版本历史
OHCI、UHCI、EHCI、xHCI详细区别
USB协议包格式
各种描述符的完整字段
USB传输底层时序
```
暂时不必深入：
```text
从零编写USB主机驱动
从零编写USB设备固件
完整USB协议栈实现
复杂USB复合设备框架
```
## 十六、一句话总结
第67章的核心不是写USB驱动，而是理解：
```text
开发板作为Host时，如何识别和使用U盘、鼠标、键盘；
开发板作为Device时，如何通过USB Gadget模拟成U盘或声卡；
OTG通过ID线决定当前接口是Host还是Device。
```
你做笔记时可以抓住这条主线：
```text
USB基础概念
    ↓
Host/Device/OTG区别
    ↓
I.MX6ULL USB硬件连接
    ↓
Linux Host驱动测试
    ↓
U盘挂载流程
    ↓
USB Gadget从机实验
```
