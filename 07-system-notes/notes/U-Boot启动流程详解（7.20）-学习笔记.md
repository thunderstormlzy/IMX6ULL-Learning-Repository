<!-- Source: U-Boot启动流程详解（7.20）.note | Exported from local Youdao database on 2026-10-06 -->

# U-Boot启动流程详解（7.20）

具体为启动UBoot然后再由bootz启动Linux内核
Uboot启动由BootROM触发，上电后CPU从BootROM指定位置（厂商规定）开始执行，BootROM会进行下列操作：
1. 读取 BOOT_MODE / BOOT_CFG 启动配置引脚
2. 判断从哪里启动：SD、EMMC、NAND、SPI NOR、USB 等
3. 去对应启动设备的固定位置读取启动镜像
4. 检查镜像头是否合法
5. 按镜像头里的信息加载代码
6. 跳转到 U-Boot 入口地址
.lds文件链接各个文件：
U-Boot 各段代码和数据怎么排布
入口地址在哪里
哪些段放前面，哪些段放后面
U-Boot 内部启动流程及作用：
reset：U-Boot 入口，CPU 开始执行后的早期启动代码。

lowlevel_init / s_init：早期底层初始化，处理 CPU、SoC、时钟、看门狗等基础配置。

_main：从汇编启动过渡到 C 语言主流程，准备栈、gd、BSS 等。

board_init_f：重定位前初始化。主要收集板子和内存信息，计算 U-Boot 之后要搬到 DDR 的哪个高地址。

relocate_code：把 U-Boot 自身复制到 DDR 高地址运行。

relocate_vectors：重定位异常向量表。

board_init_r：重定位后初始化。初始化更多外设、环境变量、命令系统，准备进入命令行或执行 bootcmd。

run_main_loop / cli_loop：进入 U-Boot 命令行循环。

cmd_process：解析并执行用户输入的 U-Boot 命令。
所需注意：
U-Boot 初始运行地址不一定是最终运行地址，U-Boot 会根据 DDR 大小、栈、malloc 区、保留区域等计算新的高地址，然后把自身搬到 DDR 高地址运行，主要目的是避免和后续加载的 Linux 内核、设备树、ramdisk 等冲突。

board_init_f：重定位前执行，环境还不完整，主要做早期初始化和重定位准备。
board_init_r：重定位后执行，环境更完整，开始初始化外设、环境变量、命令系统等。
bootz 启动 Linux 流程及作用：
补充：
（zImage：Linux 内核
dtb：设备树，描述板子硬件
bootargs：启动参数，告诉内核 console、rootfs 在哪里）
do_bootz：bootz 命令入口，解析用户传入的内核地址、initrd、设备树地址。

bootz_start：检查 zImage 是否合法，准备镜像信息。

do_bootm_states：启动 Linux 前的状态机，负责检查镜像、准备设备树、bootargs、内存等。

bootm_os_get_boot_func：根据 OS 类型选择对应启动函数。Linux 对应 do_bootm_linux。

do_bootm_linux：准备 Linux 启动参数，最后跳转到 Linux 内核入口。

在使用bootz前需要准备：
zImage：Linux 内核镜像。

dtb：设备树，告诉 Linux 当前板子的硬件信息。

bootargs：启动参数，比如 console、rootfs 位置、文件系统类型等。

内存布局：确保内核、设备树、ramdisk、U-Boot 自身不会互相覆盖。
