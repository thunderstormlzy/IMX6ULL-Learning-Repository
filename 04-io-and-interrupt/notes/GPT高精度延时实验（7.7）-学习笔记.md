<!-- Source: GPT高精度延时实验（7.7）.note | Exported from local Youdao database on 2026-10-06 -->

# GPT高精度延时实验（7.7）

用 GPT1 硬件定时器替代软件空循环 delay，提高延时精度。（时间过短的微秒计时无法实现，因为程序运行需要时间）
1. 初始化 GPT1
   -> 选择 ipg_clk = 66MHz 作为 GPT1 时钟源
   -> 设置 66 分频
   -> GPT1 实际计数频率 = 1MHz
   -> 每计数 1 次 = 1us
2. 启动 GPT1
   -> GPT1->CNT 从 0 开始向上计数
   -> 一直运行
3. delayus(us)
   -> 读取当前 GPT1->CNT，记为 oldcnt
   -> 不断读取新的 GPT1->CNT，记为 newcnt
   -> 计算 newcnt 和 oldcnt 的差值
   -> 累计经过的计数值
   -> 累计值 >= us，延时结束
4. delayms(ms)
   -> 循环调用 delayus(1000)
   -> 实现毫秒级延时
5. main.c 使用
   -> 调用 delay_init()
   -> 后续用 delayus() / delayms()
