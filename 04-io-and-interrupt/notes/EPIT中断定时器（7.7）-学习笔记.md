<!-- Source: EPIT中断定时器（7.7）.note | Exported from local Youdao database on 2026-10-06 -->

# EPIT中断定时器（7.7）

在上次中断实验中，由于需要按键消抖，因此选用EPIT中断定时器。
EPIT1 定时器倒计数-> 计数到比较值 0-> 产生 EPIT1_IRQn-> CPU 进入 start.S 的 IRQ_Handler-> system_irqhandler() 查表-> 调用 epit1_irqhandler()
GPIO 中断里不要立刻判定按键有效，而是启动 EPIT。
EPIT 延时 10ms 或 20ms 后进入 EPIT 中断。
在 EPIT 中断里重新读取按键电平，确认是否仍然按下。
确认后执行蜂鸣器/LED 翻转，再关闭 EPIT 或重新装载。
需要注意：EPIT 是倒计数定时器，GPT 是向上计数定时器，这是两者的核心差异之一。
