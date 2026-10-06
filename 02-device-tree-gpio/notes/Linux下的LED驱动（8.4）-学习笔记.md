<!-- Source: Linux下的LED驱动（8.4）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux下的LED驱动（8.4）

地址映射：MMU 负责虚拟地址到物理地址的转换；在 Linux 驱动中不能直接访问裸机物理寄存器地址，需要用 ioremap 将物理寄存器地址映射为内核虚拟地址，这样做不仅能同时运行多个进程，还能保护内核、隔离进程、按需分配内存、共享内存和统一管理设备映射。
寄存器地址映射（SW_MUX_GPIO1_IO03=ioremap(SW_MUX_GPIO1_IO03_BASE, 4)，前者是虚拟地址，后者是物理地址首地址，4为地址内存长度）
⬇
读取与写入（后续在虚拟内存中经行读写需要用到readb、writeb等操作，寄存器一般是 32 位，所以常用 readl / writel 读写；readb / writeb 是按 8 位访问）
⬇
使能GPIO（CCM_CCGR1_BASE，控制外设时钟门控）
⬇
复用GPIO使其为GPIO1_03，并进行电器属性配置（SW_MUX_GPIO1_IO03，选择复用功能；SW_PAD_GPIO1_IO03配置电气属性）
⬇
设置初始输入输出模式，高低位电平（GPIO1_GDIR，选择输入还是输出；GPIO1_DR，选择输出高电平还是低电平）
⬇
取消映射（iounmap(SW_MUX_GPIO1_IO03)释放掉所占用的虚拟地址）
修改寄存器某几位时，一般先 readl 读出原值，再修改目标位，最后 writel 写回，避免影响同一寄存器中的其他位。
