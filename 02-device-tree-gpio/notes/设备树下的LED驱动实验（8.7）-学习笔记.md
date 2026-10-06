<!-- Source: 设备树下的LED驱动实验（8.7）.note | Exported from local Youdao database on 2026-10-06 -->

# 设备树下的LED驱动实验（8.7）

8.4 的 LED 驱动直接在驱动中写死寄存器物理地址，再通过 ioremap 映射寄存器；设备树下的 LED 驱动不再把 GPIO 和引脚信息写死在驱动中，而是从设备树读取硬件资源，提高驱动复用性。
把 GPIO、pinctrl 引脚复用、电气属性等硬件资源描述，从驱动代码转移到设备树中，驱动通过 OF 函数读取设备树资源，再通过 GPIO 子系统控制对应引脚，方便在不同的设备树上使用时不需要更改驱动代码，只需要在设备树中更改所用的内容
相比于字符设备驱动开发不需要直接涉及虚拟地址，但底层 GPIO 控制器驱动仍然会完成寄存器映射
驱动流程
驱动加载 -> 查找 gpioled 节点 -> of_get_named_gpio 获取 GPIO -> gpio_is_valid 判断有效性 -> gpio_request 申请 GPIO -> gpio_direction_output 设置输出 -> gpio_set_value 控制 LED -> 卸载时 gpio_free 释放 GPIO
相关操作
of_get_named_gpio：从设备树中读取 GPIO 编号
gpio_is_valid：判断 GPIO 是否有效
gpio_request：申请 GPIO
gpio_direction_output：设置 GPIO 为输出
gpio_set_value：设置 GPIO 输出电平
gpio_free：释放 GPIO
具体内容
这是在根节点下添加的内容
	gpioled{
		#address-cells = <1>;
		#size-cells = <1>;
		compatible = "gpio-led";
		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_led>;
		gpios = <&gpio1 3 GPIO_ACTIVE_LOW>;
		status = "okay";
	};
如果驱动通过 of_find_node_by_path 查找 /gpioled 节点，则 compatible 只是描述信息；如果后续使用 platform 驱动模型，则 compatible 会用于和 of_match_table 匹配。

这是在具体的板子&iomuxc->imx6ul-evk
		pinctrl_led: ledgrp {
			fsl,pins = <
				MX6UL_PAD_GPIO1_IO03__GPIO1_IO03	0x10b0
			>;
		};
pinctrl_led 用来配置 GPIO1_IO03 的复用功能和电气属性，MX6UL_PAD_GPIO1_IO03__GPIO1_IO03 表示将该引脚复用为 GPIO 功能，0x10b0 是电气属性配置值。
