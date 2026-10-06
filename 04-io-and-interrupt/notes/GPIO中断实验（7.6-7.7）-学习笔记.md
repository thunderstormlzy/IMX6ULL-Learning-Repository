<!-- Source: GPIO中断实验（7.6-7.7）.note | Exported from local Youdao database on 2026-10-06 -->

# GPIO中断实验（7.6-7.7）

KEY0下降沿 -> GPIO1中断标志置位 -> GIC转发 -> IRQ_Handler -> system_irqhandler -> exit_irqhandler
start.S：
建立异常向量表和 IRQ_Handler，确保 CPU 发生 IRQ 时能进入统一入口；
同时保存现场、读取 GIC 中断号，再把中断交给 C 层 system_irqhandler()。
bsp/int：
初始化 GIC、设置中断向量表地址 VBAR、初始化中断函数表；
提供“注册中断处理函数”和“按中断号分发处理函数”的机制。
bsp/gpio：
在原来的 GPIO 输入/输出基础上，增加 GPIO 中断触发方式配置；
提供 GPIO 中断使能、禁止、标志位清除等底层操作。
bsp/exit：
针对本实验的 KEY0，配置 GPIO1_IO18 为下降沿中断；
使能对应 GIC 中断，注册 KEY0 的中断处理函数，并在处理函数里完成蜂鸣器翻转和清标志。
注意：一定要在中断结束时清除中断标志位，中断标志位(GPIO_ISR)不清会重复进入中断函数
