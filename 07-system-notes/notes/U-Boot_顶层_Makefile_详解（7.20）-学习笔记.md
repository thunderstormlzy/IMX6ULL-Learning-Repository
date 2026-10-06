<!-- Source: U-Boot 顶层 Makefile 详解（7.20）.note | Exported from local Youdao database on 2026-10-06 -->

# U-Boot 顶层 Makefile 详解（7.20）

makefile主要流程：
configs/xxx_defconfig➡make xxx_defconfig➡生成 .config➡make ARCH=arm CROSS_COMPILE=xxx-➡根据 .config 编译源码➡
生成 u-boot / u-boot.bin / u-boot.imx
目录内的主要内容：
configs/
存放默认配置文件 xxx_defconfig。

board/
存放具体开发板相关代码，比如 DDR、网卡、板级初始化。

include/configs/
存放板级配置头文件，比如启动地址、bootcmd、bootargs 等。

arch/arm/
存放 ARM 架构相关代码，包括启动代码、SoC 相关代码。

arch/arm/dts/
存放设备树文件，用于描述板子硬件。

drivers/
存放外设驱动，比如 MMC、网卡、I2C、SPI、USB 等。

cmd/
存放 U-Boot 命令入口，比如 mmc、fatload、bootz、tftp。

common/
存放 U-Boot 通用核心逻辑。
关键概念：
make xxx_defconfig 不是编译源码，而是生成 .config。

configs/xxx_defconfig 是默认配置模板。

.config 是最终实际参与编译的完整配置文件。

ARCH=arm 指定目标架构是 ARM。

CROSS_COMPILE=arm-linux-gnueabihf- 指定交叉编译工具链前缀。

真正 make 时，U-Boot 根据 .config 决定哪些源码参与编译。
编译产物区别：
u-boot：ELF 文件，带符号信息，常用于调试分析。

u-boot.bin：纯二进制 U-Boot 镜像。

u-boot.imx：I.MX6ULL 可启动镜像，在 u-boot.bin 前加入了 I.MX 启动头/DCD 等信息。
实际烧写到 SD/EMMC 时通常用 u-boot.imx。
排错思路：
命令没出现：优先查 .config / defconfig / Kconfig。

命令存在但执行异常：先看 cmd/ 命令入口，再顺着看 fs/、drivers/、board/。

外设识别不到：看设备树、板级初始化、驱动配置。

编译不过：看 ARCH、CROSS_COMPILE、defconfig、工具链路径。
