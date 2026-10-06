<!-- Source: C语言led驱动（7.1）.note | Exported from local Youdao database on 2026-10-06 -->

# C语言led驱动（7.1）

流程：
start.s -> 设置CPU模式 -> 设置SP -> main -> 开时钟 -> 配IOMUXC -> 配GPIO方向 -> 控制DR
volatile ：禁止对其进行优化
主要内容
start.s：配置处理器进入SVC（管理员调用）模式，设置sp指针地址，跳转到main函数
.global _start          @ 声明 _start 为全局符号，链接器会将其作为程序的入口点
_start:                 @ 程序入口标签
    mrs r0, cpsr        @ 将当前程序状态寄存器（CPSR）的值读取到通用寄存器 r0
                        @ CPSR 保存了当前处理器模式、中断禁用状态、条件标志等
    bic r0, r0, #0x1f   @ 将 r0 中的低 5 位清零（按位清除）
                        @ 低 5 位（bit0~bit4）表示处理器模式位（M[4:0]）
                        @ 这步操作是为了清除当前模式，准备切换到新模式
    orr r0, r0, #0x13   @ 将 r0 的低 5 位设置为 0x13（二进制 10011）
                        @ 0x13 对应 SVC（超级用户）模式，即管理员模式
                        @ 该模式拥有最高权限，可访问所有系统资源
    msr cpsr, r0        @ 将修改后的值写回 CPSR，完成模式切换
                        @ 此时 CPU 切换到 SVC 模式，并保留其他标志位不变
    ldr sp, =0x80200000 @ 将堆栈指针（SP）设置为 0x80200000
                        @ 这是一个内存地址，通常位于 DDR/SDRAM 中
                        @ 为后续的函数调用（如跳转到 main）准备栈空间
    b main              @ 无条件跳转到 main 函数（C 语言入口）
                        @ 程序从此处开始执行 C 语言代码
main.h：定义mian.c中涉及到的相关寄存器地址

main.c：初始化寄存器，进行代码编译
<>和" "前者是系统的头文件，后者是从当前目录搜索，若搜索不到再搜索系统头文件
#include "main.h"          // 包含头文件（定义了寄存器地址映射等）
/**
 * @brief 使能所有外设时钟
 * @note  CCM（Clock Controller Module）时钟控制模块
 *        CCGR（Clock Gate Control Register）时钟门控寄存器
 *        设置所有位为 1，表示使能对应外设的时钟
 */
void clk_enable(void)
{
    CCM_CCGR0 = 0xffffffff;   // 使能 CCGR0 控制的外设时钟
    CCM_CCGR1 = 0xffffffff;   // 使能 CCGR1 控制的外设时钟
    CCM_CCGR2 = 0xffffffff;   // 使能 CCGR2 控制的外设时钟
    CCM_CCGR3 = 0xffffffff;   // 使能 CCGR3 控制的外设时钟
    CCM_CCGR4 = 0xffffffff;   // 使能 CCGR4 控制的外设时钟
    CCM_CCGR5 = 0xffffffff;   // 使能 CCGR5 控制的外设时钟
    CCM_CCGR6 = 0xffffffff;   // 使能 CCGR6 控制的外设时钟
    （GPIO1 时钟在此寄存器中）
}
/**
 * @brief LED 初始化（GPIO1_IO03 引脚）
 * @note  开发板上的 LED 通常连接到 GPIO1_IO03
 *        步骤：复用功能配置 → PAD属性配置 → 方向配置 → 初始状态设置
 */
void led_init(void)
{
    /* 步骤1：配置引脚复用功能 */
    SW_MUX_GPIO1_IO03 = 0x5;   // 设置 ALT5 模式，将 GPIO1_IO03 复用为 GPIO 功能
                               // 0x5 = 二进制 0101，对应 GPIO 模式
    /* 步骤2：配置 PAD 电气属性（驱动能力、上下拉、速度等） */
    SW_PAD_GPIO1_IO03 = 0x10b0; // 设置压摆率、驱动强度、上下拉等参数
                                // 0x10b0 是常用配置值（具体位含义见数据手册）
    /* 步骤3：配置 GPIO 方向为输出 */
    GPIO1_GDIR = 0x8;          // GDIR（GPIO Direction Register）
                               // bit3 置 1，表示 GPIO1_IO03 为输出模式
                               // 0x8 = 二进制 1000
    /* 步骤4：设置初始输出状态 */
    GPIO1_DR = 0x0;            // DR（Data Register）数据寄存器
                               // bit3 清零，初始输出低电平（LED 点亮）
                               // 注意：此处注释写"灯泡初始状态为开启"（低电平有效）
}
/**
 * @brief 短延时函数（微秒级）
 * @param time  延时计数（循环次数）
 * @note  通过简单的 CPU 循环消耗时间，非精确延时
 */
void delay_short(volatile unsigned int time)
{
    while(time--);            // 空循环，volatile 防止被编译器优化掉
}
/**
 * @brief 延时函数（毫秒级）
 * @param time  延时计数（循环次数）
 * @note  通过多次调用 delay_short 实现较长时间延时
 */
void delay(volatile unsigned int time)
{
    while(time--)             // 外层循环 time 次
    {
        delay_short(0x7ff);   // 每次内层循环 0x7ff（2047）次
    }
}
/**
 * @brief LED 点亮
 * @note  GPIO1_DR bit3 清零 → 输出低电平
 *        如果 LED 是低电平点亮（共阳极接法），则 LED 亮
 */
void led_on(void)
{
    GPIO1_DR &= ~(1 << 3);    // DR bit3 清零：~(1<<3) = ~0x08 = 0xFFFFFFF7
                              // 与操作保持其他位不变，仅清除 bit3
}
/**
 * @brief LED 熄灭
 * @note  GPIO1_DR bit3 置 1 → 输出高电平
 *        如果 LED 是低电平点亮（共阳极接法），则 LED 灭
 */
void led_off(void)
{
    GPIO1_DR |= (1 << 3);     // DR bit3 置 1：1<<3 = 0x08
                              // 或操作保持其他位不变，仅设置 bit3
}
/**
 * @brief 主函数（程序入口）
 * @note  执行流程：
 *        1. 使能所有外设时钟
 *        2. 初始化 LED（GPIO 引脚）
 *        3. 无限循环：LED 亮 → 延时 → LED 灭 → 延时
 */
int main(void)
{
    clk_enable();              // 使能时钟（必须最先执行，否则寄存器无法操作）
    led_init();                // 初始化 LED 引脚
    while(1)                   // 主循环
    {
        led_on();              // LED 点亮
        delay(500);            // 延时约 500ms（具体时间取决于 CPU 频率）
        led_off();             // LED 熄灭
        delay(500);            // 延时约 500ms
    }
    return 0;                  // 永远不会执行到这里
}
Makefile：方便进行编译后的链接，修改单个内容后方便重新生成bin文件
*start.s要放在编译的最前面*
# ============================================================
# 目标文件列表（由汇编文件和C文件编译生成的 .o 文件）
# ============================================================
objts = start.o main.o          # 依赖的目标文件（.o 文件）
                                # start.o 来自 start.s（启动代码）
                                # main.o 来自 main.c（主程序）
# ============================================================
# 最终目标：生成 LED 的二进制镜像文件
# 依赖关系：ledc.bin 依赖于所有 .o 文件
# ============================================================
ledc.bin: $(objts)
    # 步骤1：链接所有 .o 文件，生成 ELF 格式的可执行文件
    # -T imx6u.lds  ：指定链接脚本（定义内存布局、段地址等）
    # $^            ：自动变量，代表所有依赖文件（即 start.o main.o）
    # -o ledc.elf   ：指定输出文件名为 ledc.elf
    arm-linux-gnueabihf-ld -Timx6u.lds $^ -o ledc.elf 
    # 步骤2：将 ELF 文件转换为纯二进制镜像文件（用于烧录）
    # -O binary     ：输出格式为二进制（raw binary）
    # -S            ：移除调试符号信息（减小文件大小）
    # ledc.elf      ：输入文件（上一步生成的 ELF 文件）
    # $@            ：自动变量，代表目标文件（即 ledc.bin）
    arm-linux-gnueabihf-objcopy -O binary -S ledc.elf $@
# ============================================================
# 规则：将 .c 文件编译为 .o 文件
# 模式匹配：%.o 匹配所有以 .o 结尾的文件，%.c 匹配对应的 .c 文件
# ============================================================
%.o : %.c
    # -Wall          ：显示所有警告信息（帮助发现潜在问题）
    # -nostdlib      ：不使用标准 C 库（裸机程序无需标准库）
    # -c             ：只编译不链接（生成 .o 目标文件）
    # -o $@          ：指定输出文件（$@ 代表目标文件名）
    # $<             ：自动变量，代表第一个依赖文件（即对应的 .c 文件）
    arm-linux-gnueabihf-gcc -Wall -nostdlib -c -o $@ $<
# ============================================================
# 规则：将 .s 文件（汇编文件）编译为 .o 文件
# 模式匹配：%.o 匹配所有以 .o 结尾的文件，%.s 匹配对应的 .s 文件
# ============================================================
%.o : %.s
    # -Wall          ：显示所有警告信息
    # -nostdlib      ：不使用标准 C 库
    # -c             ：只编译不链接（生成 .o 目标文件）
    # -o $@          ：指定输出文件（$@ 代表目标文件名）
    # $<             ：自动变量，代表第一个依赖文件（即对应的 .s 文件）
    arm-linux-gnueabihf-gcc -Wall -nostdlib -c -o $@ $<
# ============================================================
# 清理规则：删除所有编译生成的文件
# ============================================================
clean:
    # rm -f          ：强制删除文件（不提示确认）
    # *.o            ：删除所有目标文件
    # *.elf          ：删除所有 ELF 可执行文件
    # *.bin          ：删除所有二进制镜像文件
    rm -f *.o *.elf *.bin
