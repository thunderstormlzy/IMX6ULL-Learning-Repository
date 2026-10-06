<!-- Source: Linux内核移植（7.24  7.25）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux内核移植（7.24  7.25）

Boot ROM➡U-Boot➡通过tftpboot加载 zImage 和 dtb➡bootz 启动 Linux 内核➡Linux 根据 dtb 初始化硬件➡Linux 根据 bootargs 挂载 rootfs➡执行 /sbin/init（初始化用户空间）➡进入用户空间
其中：
U-Boot：负责启动 Linux
zImage：Linux 内核镜像
dtb：设备树，描述板子硬件
bootargs：U-Boot 传给 Linux 的启动参数
rootfs：Linux 用户空间根文件系统
以上是Linux的启动流程，Linux的内核移植和U-Boot类似
defconfig → .config → make zImage → make dtbs
zImage 是 Linux 内核
dtb 是设备树二进制文件
相关文件的内容：
arch/arm/
ARM 架构相关代码。
arch/arm/boot/
zImage 输出目录。
arch/arm/boot/dts/
设备树源码和 dtb 输出目录。
drivers/
各种驱动，比如网卡、I2C、SPI、MMC、LCD、USB。
fs/
文件系统代码。
init/
Linux 启动初始化代码。
kernel/
内核核心代码，比如调度、中断、进程。
mm/
内存管理。
net/
网络协议栈。
scripts/
编译脚本和主机工具，比如 dtc。
常用查找思路：
改设备树：arch/arm/boot/dts/
改驱动：drivers/
改内核配置：.config / Kconfig / defconfig
看启动流程：init/
bootargs 是 U-Boot 传给 Linux 的启动参数。
常见写法：
setenv bootargs 'console=ttymxc0,115200 root=/dev/mmcblk1p2 rootwait rw'
saveenv
含义：
console=ttymxc0,115200
Linux 控制台使用 UART，波特率 115200。
root=/dev/mmcblk1p2
根文件系统位于 mmcblk1 的第 2 分区。
rootwait
等待根设备初始化完成。
rw
根文件系统以读写方式挂载。
注意：
U-Boot 从 SD 启动，不代表 rootfs 必须在 SD。
SD 启动 U-Boot，Linux 仍然可以从 eMMC 挂载 rootfs。
设备树的作用是描述硬件。
Linux 通过 dtb 知道：外设基地址、引脚复用、中断号、时钟、GPIO、PHY 地址、屏幕参数、MMC/eMMC 参数
新增自己的开发板时，一般需要：
1. 复制原设备树文件
2. 修改设备树名称
3. 修改 arch/arm/boot/dts/Makefile
4. 编译 dtbs
5. U-Boot bootcmd 加载自己的 dtb
设备树文件通常在：
arch/arm/boot/dts/
