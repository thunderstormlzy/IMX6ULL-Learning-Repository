# Embedded Linux Driver Portfolio

基于 i.MX6ULL 的 Linux 驱动学习仓库，面向嵌入式 Linux、BSP 和底层软件岗位。

## 内容结构

| 目录 | 重点能力 |
| --- | --- |
| `01-char-device` | 字符设备、设备号、用户态测试程序 |
| `02-device-tree-gpio` | 设备树、pinctrl、GPIO 子系统 |
| `03-concurrency` | 原子操作、自旋锁、信号量、互斥体 |
| `04-io-and-interrupt` | 按键中断、内核定时器、阻塞/非阻塞 IO、异步通知 |
| `05-platform-input` | platform 总线、设备树匹配、MISC、input 子系统 |
| `06-bus-drivers` | I2C、SPI、多点触摸驱动 |
| `07-system-notes` | U-Boot、Linux 启动、内核移植、根文件系统、多媒体笔记 |
| `08-bare-metal-notes` | 裸机启动、SDK、时钟和 BSP 学习笔记 |

## 使用说明

这些示例面向 i.MX6ULL Linux 4.1.15 开发环境。每个练习目录保留源码和原始 Makefile；编译前请根据本机环境修改 `KERNELDIR`、交叉编译器和目标内核配置。
