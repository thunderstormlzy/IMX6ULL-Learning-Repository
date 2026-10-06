<!-- Source: Linux设备树（8.6）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux设备树（8.6）

由于早期 ARM Linux 中每块板子的硬件信息都硬编码在内核源码中，导致内核源码中存在大量冗余的 board-xxx.c 文件。为了将硬件描述从内核中解耦，并让同一份内核镜像能够支持多种不同的硬件平台，Linux 引入了设备树（Device Tree）机制。
通过 DTSI（公共描述） 和 DTS（具体描述） 的继承结构，实现了芯片级硬件描述的复用和板级硬件配置的定制，有效减少了冗余。
可以理解为，dts = dtsi + 本板特有内容。
DTSI：设备树的"公共头文件"，它是公共的、可复用的硬件描述
DTS：设备树源文件，它是 Linux 内核中用来描述硬件配置的一种文本格式文件，是具体的、完整的板级描述
DTC：设备树的编译器
.dtsi文件的引用可以是.h也可以是.dtsi，其用于描述SOC 的内部外设信息

设备节点用来描述硬件设备或硬件控制器的具体信息

根节点表示设备树的起始

节点是由一堆的属性组成，节点都是具体的设备，不同的设备需要的属性不同，用户可以自定义属性。
compatible
解释：表示这个设备节点和哪些硬件/驱动兼容。
用处：驱动通过它和设备树节点匹配。
model
解释：表示板子或设备的型号名称。
用处：描述整块板子的身份，便于识别和调试。
status
解释：表示该节点是否启用。
用处：控制设备节点参与不参与内核初始化。
reg
解释：表示设备使用的地址和长度资源。
用处：告诉驱动外设寄存器在哪里，占多大空间。
#address-cells
解释：表示子节点地址部分占几个 cell。
用处：规定子节点 reg 里地址字段的格式。
#size-cells
解释：表示子节点长度部分占几个 cell。
用处：规定子节点 reg 里长度字段的格式。
name
解释：表示节点名字。
用处：标识节点名称，便于查看和解析。
device_type
解释：表示节点对应的设备类型。
用处：让内核识别这是什么类别的硬件。
ranges
解释：表示父总线和子总线之间的地址映射关系。
用处：帮助内核把子节点地址转换成可访问地址。
interrupts
解释：表示设备使用的中断信息。
用处：告诉内核这个设备的中断号和触发方式。
interrupt-parent
解释：表示当前节点对应的中断控制器。
用处：说明 interrupts 应由哪个中断控制器解释。
aliases
解释：表示设备节点的别名。
用处：给设备提供固定、简短的引用名称。
chosen
解释：表示启动时内核要使用的被选中信息。
用处：传递启动参数、控制台等启动相关配置。
以下是具体例子
/ {
    model = "Freescale i.MX6ULL ALPHA Board";
    compatible = "fsl,imx6ull-alpha", "fsl,imx6ull";
    #address-cells = <1>;
    #size-cells = <1>;
    aliases {
        serial0 = &uart1;
    };
    chosen {
        bootargs = "console=ttymxc0,115200";
    };
    soc {
        ranges;
        gpio1: gpio@0209c000 {
            compatible = "fsl,imx6ul-gpio";
            reg = <0x0209c000 0x4000>;
            interrupts = <66>;
            interrupt-parent = <&intc>;
            status = "okay";
        };
    };
};
model 描述这是什么板子。
compatible 说明这块板子兼容 imx6ull-alpha 和 imx6ull，驱动或内核可以据此匹配。
#address-cells = <1> 表示子节点 reg 里的地址用 1 个 32 位数据表示。
#size-cells = <1> 表示子节点 reg 里的长度用 1 个 32 位数据表示。
aliases 里 serial0 = &uart1，表示给 uart1 起一个 serial0 的别名。
chosen 里的 bootargs 是传给 Linux 内核的启动参数。
soc 节点表示 SoC 内部外设总线。
ranges 表示 soc 子节点地址和父地址空间存在映射关系（父节点负责组织和提供规则，子节点负责描述具体硬件）。
gpio1 节点描述 GPIO1 控制器。
reg = <0x0209c000 0x4000> 表示 GPIO1 寄存器起始地址是 0x0209c000，长度是 0x4000。
interrupts 表示 GPIO1 使用的中断信息。
interrupt-parent 表示这个中断信息由 intc 这个中断控制器解释。
status = "okay" 表示 GPIO1 节点启用。
除了根节点以外还有部分特殊节点：
aliases 子节点：为了访问节点时更加方便，使用aliases 子节点来定义别名
chosen 节点：主要是为了 uboot 向 Linux 内核传递数据，重点是 bootargs 参数（由uboot添加给chosen，让引导程序和内核有一个固定地方传启动参数）
常用OF操作函数
`of_find_node_by_path(const char *path)`  
作用：根据设备树节点的完整路径查找节点。  
参数：`path` 表示节点路径，例如 `/alphaled`。
`of_find_node_by_name(struct device_node *from, const char *name)`  
作用：根据节点名字查找设备树节点。  
参数：`from` 表示从哪个节点开始查找，填 `NULL` 表示从根节点开始；`name` 表示要查找的节点名。
`of_find_compatible_node(struct device_node *from, const char *type, const char *compatible)`  
作用：根据 `compatible` 属性查找匹配的设备树节点。  
参数：`from` 表示从哪个节点开始查找；`type` 表示设备类型，一般可填 `NULL`；`compatible` 表示要匹配的 compatible 字符串。
`of_property_read_u32(const struct device_node *np, const char *propname, u32 *out_value)`  
作用：读取节点中一个 32 位整数属性。  
参数：`np` 表示设备树节点；`propname` 表示属性名；`out_value` 用来保存读取到的值。
`of_property_read_u32_array(const struct device_node *np, const char *propname, u32 *out_values, size_t sz)`  
作用：读取节点中多个 32 位整数属性。  
参数：`np` 表示设备树节点；`propname` 表示属性名；`out_values` 用来保存读取到的数组；`sz` 表示要读取的元素个数。
`of_property_read_string(const struct device_node *np, const char *propname, const char **out_string)`  
作用：读取节点中的字符串属性。  
参数：`np` 表示设备树节点；`propname` 表示属性名；`out_string` 用来保存读取到的字符串指针。
`of_property_read_bool(const struct device_node *np, const char *propname)`  
作用：判断节点中是否存在某个属性。  
参数：`np` 表示设备树节点；`propname` 表示要判断的属性名。
`of_iomap(struct device_node *np, int index)`  
作用：根据节点的 `reg` 属性完成地址映射，返回内核可访问的虚拟地址。  
参数：`np` 表示设备树节点；`index` 表示映射第几组 `reg` 地址资源。
`of_get_named_gpio(struct device_node *np, const char *propname, int index)`  
作用：从节点指定属性中获取 GPIO 编号。  
参数：`np` 表示设备树节点；`propname` 表示 GPIO 属性名；`index` 表示获取该属性中的第几个 GPIO。
`irq_of_parse_and_map(struct device_node *dev, int index)`  
作用：从设备树节点中解析中断信息，并转换成 Linux 内核使用的中断号。  
参数：`dev` 表示设备树节点；`index` 表示获取第几个中断。
`of_node_put(struct device_node *node)`  
作用：释放设备树节点引用。  
参数：`node` 表示需要释放引用的设备树节点。
