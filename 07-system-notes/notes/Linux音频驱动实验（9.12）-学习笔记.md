<!-- Source: Linux音频驱动实验（9.12）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux音频驱动实验（9.12）

# WM8960音频驱动实验笔记
## 一、WM8960的作用
WM8960是音频编解码芯片，内部包含：
- ADC：把麦克风的模拟声音转换为数字数据，用于录音。
- DAC：把数字音频转换为模拟信号，用于播放。
- 麦克风放大器。
- 耳机放大器。
- 喇叭功放。
- 音量控制、输入选择和混音功能。
I.MX6ULL本身主要负责数字音频数据处理，不能直接完成完整的麦克风采集和喇叭驱动，所以需要WM8960配合。
## 二、音频数据传输方式
本实验使用两种接口：
```text
I2C：配置WM8960内部寄存器
I2S：传输真正的音频数据
```
两者作用不同：
```text
I2C负责告诉WM8960怎么工作
I2S负责在I.MX6ULL和WM8960之间传输音频数据
```
播放流程：
```text
应用程序
    ↓
ALSA
    ↓
I.MX6ULL SAI2
    ↓ I2S
WM8960 DAC
    ↓
耳机或喇叭
```
录音流程：
```text
麦克风
    ↓
WM8960 ADC
    ↓ I2S
I.MX6ULL SAI2
    ↓
ALSA
    ↓
录音文件
```
## 三、I2S主要信号
WM8960和I.MX6ULL之间主要使用以下信号：
- `MCLK`：主时钟，给WM8960提供工作时钟。
- `BCLK`：位时钟，每传输1位数据对应一个时钟。
- `LRCK/ADCLRC/DACLRC`：左右声道选择信号，也叫帧时钟。
- `DACDAT`：I.MX6ULL发送给WM8960的数据，用于播放。
- `ADCDAT`：WM8960发送给I.MX6ULL的数据，用于录音。
常用计算公式：
```text
LRCK = 采样率
BCLK = 采样率 × 声道数 × 采样位数
MCLK = 采样率 × 256 或 采样率 × 384
```
例如：
```text
采样率为44.1kHz
采样位数为16位
双声道
BCLK = 44100 × 2 × 16 = 1.4112MHz
```
本章不需要深入分析每个SAI寄存器，但要理解SAI2负责I2S音频数据和时钟传输。
## 四、硬件连接需要确认的内容
使用WM8960时，必须确认：
- WM8960挂在哪个I2C控制器上。
- WM8960的I2C地址。
- 使用哪个SAI接口。
- SAI引脚是否配置正确。
- 麦克风连接到哪个输入通道。
- 耳机和喇叭连接到哪个输出通道。
WM8960常见I2C地址为：
```text
0x1A
```
底板版本可能不同：
```text
V2.4以前：WM8960通常连接I2C2
部分新版本：可能连接I2C1
```
因此，不能只按照教程文字判断，应该以你的底板原理图和当前设备树为准。
## 五、设备树的主要组成
WM8960音频设备树主要由以下几部分组成：
```text
1. I2C中的WM8960节点
2. SAI2节点
3. SAI2引脚复用节点
4. sound声卡节点
```
### 1. WM8960节点
示例：
```dts
& i2c2 {
    codec: wm8960@1a {
        compatible = "wlf,wm8960";
        reg = <0x1a>;
        clocks = <&clks IMX6UL_CLK_SAI2>;
        clock-names = "mclk";
        wlf,shared-lrclk;
    };
};
```
主要作用：
- `compatible`：匹配Linux中的WM8960驱动。
- `reg`：表示WM8960的I2C地址。
- `clocks`：指定WM8960使用的主时钟。
- `clock-names`：说明该时钟叫`mclk`。
- `wlf,shared-lrclk`：表示播放和录音共用左右声道帧时钟。
### 2. SAI2节点
```dts
&sai2 {
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_sai2>;
    assigned-clocks = <&clks IMX6UL_CLK_SAI2_SEL>,
                      <&clks IMX6UL_CLK_SAI2>;
    assigned-clock-parents = <&clks IMX6UL_CLK_PLL4_AUDIO_DIV>;
    assigned-clock-rates = <0>, <11289600>;
    status = "okay";
};
```
作用：
- `pinctrl-0`：使用SAI2的引脚配置。
- `assigned-clock-parents`：选择SAI2时钟来源。
- `assigned-clock-rates`：设置SAI2时钟频率。
- `status = "okay"`：启用SAI2。
### 3. SAI2引脚配置
```dts
pinctrl_sai2: sai2grp {
    fsl,pins = <
        MX6UL_PAD_JTAG_TDI__SAI2_TX_BCLK 0x17088
        MX6UL_PAD_JTAG_TDO__SAI2_TX_SYNC 0x17088
        MX6UL_PAD_JTAG_TRST_B__SAI2_TX_DATA 0x11088
        MX6UL_PAD_JTAG_TCK__SAI2_RX_DATA 0x11088
        MX6UL_PAD_JTAG_TMS__SAI2_MCLK 0x17088
    >;
};
```
每一项通常由两部分组成：
```text
引脚复用功能 + 电气属性
```
例如：
```dts
MX6UL_PAD_JTAG_TDI__SAI2_TX_BCLK 0x17088
```
表示：
- 将JTAG_TDI引脚复用为SAI2发送位时钟。
- `0x17088`配置该引脚的上下拉、驱动能力、速度等电气属性。
### 4. sound节点
```dts
sound {
    compatible = "fsl,imx6ul-evk-wm8960",
                 "fsl,imx-audio-wm8960";
    model = "wm8960-audio";
    cpu-dai = <&sai2>;
    audio-codec = <&codec>;
    asrc-controller = <&asrc>;
    codec-master;
    gpr = <&gpr 4 0x100000 0x100000>;
    audio-routing = 
        "Headphone Jack", "HP_L",
        "Headphone Jack", "HP_R",
        "Ext Spk", "SPK_LP",
        "Ext Spk", "SPK_LN",
        "Ext Spk", "SPK_RP",
        "Ext Spk", "SPK_RN";
};
```
主要作用：
- `compatible`：匹配I.MX6ULL和WM8960组成的声卡驱动。
- `model`：设置声卡名称。
- `cpu-dai`：指定CPU端的音频接口，这里是SAI2。
- `audio-codec`：指定外部音频编解码器。
- `asrc-controller`：指定异步采样率转换模块。
- `codec-master`：设置WM8960作为音频时钟主设备。
- `gpr`：配置I.MX6ULL相关音频复用或时钟控制。
- `audio-routing`：描述声卡内部的音频信号连接关系。
## 六、audio-routing的作用
`audio-routing`不是寄存器地址，而是描述音频信号的连接路径。
例如：
```dts
"Headphone Jack", "HP_L",
```
表示：
```text
WM8960的HP_L输出连接到耳机插孔
```
```dts
"Ext Spk", "SPK_LP",
```
表示：
```text
WM8960的SPK_LP输出连接到外部喇叭
```
```dts
"LINPUT2", "Mic Jack",
```
表示：
```text
Mic Jack连接到WM8960的LINPUT2输入
```
如果`audio-routing`配置错误，可能出现：
- 声卡能够识别，但没有声音。
- 播放正常，录音没有数据。
- 耳机有声音，喇叭没有声音。
- 只能左声道录音。
- 某个输入源无法使用。
## 七、内核驱动配置
主要使能以下内容：
```text
ALSA
ALSA SoC
I.MX SoC音频支持
ASRC支持
I.MX板级WM8960音频驱动
WM8960 CODEC驱动
```
重点配置路径：
```text
Device Drivers
    -> Sound card support
        -> Advanced Linux Sound Architecture
            -> ALSA for SoC audio support
                -> SoC Audio for Freescale CPUs
```
教程还要求取消：
```text
OSS Mixer API
OSS PCM (digital audio) API
```
本实验使用ALSA，不依赖OSS。
配置完成以后重新编译：
```bash
make zImage -j8
make dtbs
```
然后使用新的内核和设备树启动开发板。
## 八、启动后的驱动流程
系统启动后大致经过以下流程：
```text
1. 内核读取设备树
2. 匹配I2C中的WM8960节点
3. 加载WM8960 CODEC驱动
4. 初始化WM8960
5. 初始化I.MX6ULL的SAI2
6. 加载I.MX音频声卡驱动
7. 连接CPU DAI和CODEC DAI
8. 创建ALSA声卡设备
9. 生成播放和录音设备
```
检查声卡：
```bash
aplay -l
arecord -l
```
如果能看到WM8960对应的声卡，说明设备树、SAI、I2C和音频驱动基本已经连接成功。
## 九、ALSA各工具的作用
```text
alsa-lib：用户空间音频库
alsa-utils：音频测试和配置工具
aplay：播放音频
arecord：录音
amixer：命令行设置声卡
alsactl：保存和恢复声卡配置
alsamixer：图形化配置声卡
```
关系可以理解为：
```text
aplay/arecord/amixer
        ↓
    alsa-lib
        ↓
      ALSA
        ↓
    内核音频驱动
```
## 十、播放测试流程
第一次播放前，需要打开输出通路并设置音量：
```bash
amixer sset Headphone 100,100
amixer sset Speaker 1 20,120
amixer sset 'Right Output Mixer PCM' on
amixer sset 'Left Output Mixer PCM' on
```
播放WAV文件：
```bash
aplay test.wav
```
如果没有声音，依次检查：
```text
1. WM8960是否被识别
2. aplay -l是否能看到声卡
3. Headphone或Speaker音量是否为0
4. 左右Output Mixer是否打开
5. audio-routing是否正确
6. 喇叭、耳机和音频线连接是否正确
```
## 十一、MIC录音测试流程
WM8960录音前，需要配置输入通道：
```bash
amixer cset name='Capture Volume' 90,90
amixer sset 'ADC PCM' 200
```
然后根据硬件连接打开对应输入通道。
录音命令：
```bash
arecord -f cd -d 10 record.wav
```
参数含义：
```text
-f cd：使用CD质量格式
-d 10：录音10秒
record.wav：保存的录音文件
```
录音完成以后播放：
```bash
aplay record.wav
```
如果录音没有声音，重点检查：
```text
1. Capture Volume是否打开
2. 对应输入通道是否打开
3. 输入混音器是否配置正确
4. 麦克风是否接在正确声道
5. audio-routing是否描述正确
```
## 十二、为什么MIC录音可能只有左声道
ALPHA开发板上的板载MIC通常只连接到WM8960的左声道输入，因此录音时可能出现：
```text
左声道有声音
右声道没有声音
```
这不一定是驱动错误，而可能是硬件连接本来就只有一个声道。
如果想让左右声道都使用左侧MIC的数据，可以修改WM8960的R23寄存器，将左右ADC数据都设置为左ADC数据。
默认值：
```text
0x01C0
```
修改为：
```text
0x01C4
```
这样得到的左右声道内容实际上是相同的，并不是真正的立体声。
## 十三、Line In和MIC的区别
```text
MIC：麦克风输入，通常是单声道，信号较弱，需要放大
Line In：线路输入，通常是双声道，信号强度较高
```
Line In不能直接接普通话筒，否则可能出现声音很小或无法正常录音。
Line In测试一般使用：
```text
手机或电脑音频输出
        ↓
3.5mm公对公音频线
        ↓
开发板Line In接口
```
## 十四、开机自动恢复声卡配置
声卡的音量和输入输出通路通常不会自动保存。
先手动配置声卡，然后保存：
```bash
mkdir /var/lib/alsa -p
alsactl -f /var/lib/alsa/asound.state store
```
恢复配置：
```bash
alsactl -f /var/lib/alsa/asound.state restore
```
可以在`/etc/init.d/rcS`中加入：
```bash
if [ -f "/var/lib/alsa/asound.state" ]; then
    echo "ALSA: Restoring mixer setting......"
    /sbin/alsactl -f /var/lib/alsa/asound.state restore &
fi
```
系统启动时就会自动恢复音量和通路设置。
## 十五、本章最重要的结论
```text
WM8960负责模拟音频处理
I2C负责配置WM8960寄存器
SAI负责I2S音频数据传输
I2S负责传输数字音频数据
ASoC负责将CPU音频接口和CODEC组成声卡
ALSA负责给用户空间提供统一音频接口
aplay负责播放
arecord负责录音
amixer负责设置音量和音频通路
```
整个实验主流程：
```text
确认底板和WM8960连接关系
        ↓
确认WM8960的I2C总线和地址
        ↓
配置WM8960设备树节点
        ↓
使能SAI2和SAI2引脚
        ↓
配置sound声卡节点
        ↓
使能ALSA、ASoC和WM8960驱动
        ↓
编译内核和设备树
        ↓
启动开发板
        ↓
使用aplay -l、arecord -l检查声卡
        ↓
使用amixer打开音频通路
        ↓
使用aplay播放或arecord录音
        ↓
使用alsactl保存声卡配置
```
SAI全称是 **Synchronous Audio Interface**，中文叫**同步音频接口**。它是 I.MX6ULL 内部的一个音频外设，用来连接CPU和WM8960这样的音频编解码芯片。
可以简单理解为：
```text
I2C控制器：负责I2C通信
SPI控制器：负责SPI通信
SAI控制器：负责I2S音频数据通信
```
在你的WM8960实验中，I.MX6ULL使用的是`SAI2`。
**SAI的主要作用**
1. 产生和接收音频时钟。
2. 发送播放数据。
3. 接收录音数据。
4. 按照I2S格式组织左右声道数据。
5. 通过FIFO暂存音频数据，减少CPU频繁操作。
**播放时：**
```text
Linux应用程序
    ↓
ALSA
    ↓
SAI2
    ↓ I2S
WM8960的DAC
    ↓
耳机或喇叭
```
SAI2把Linux传下来的数字音频数据，通过I2S发送给WM8960。WM8960再通过DAC转换成模拟声音。
**录音时：**
```text
麦克风
    ↓
WM8960的ADC
    ↓ I2S
SAI2
    ↓
ALSA
    ↓
录音文件
```
WM8960把采集到的声音转换成数字数据，再通过I2S发送给SAI2，SAI2接收后交给Linux。
**SAI和I2C的区别**
```text
I2C：配置WM8960内部寄存器
SAI：传输真正的音频数据
```
例如：
```text
通过I2C设置WM8960音量、输入通道、输出通道
通过SAI发送音乐数据或接收录音数据
```
所以，I2C更像是“设置参数”，SAI更像是“传输音频”。
**SAI使用的主要信号**
- `MCLK`：主时钟，给WM8960提供工作时钟。
- `BCLK`：位时钟，每传输1位数据对应一个时钟。
- `LRCK`或`WS`：左右声道选择信号。
- `TX_DATA`：I.MX6ULL发送给WM8960的数据，用于播放。
- `RX_DATA`：I.MX6ULL接收WM8960的数据，用于录音。
在你的设备树中类似：
```dts
MX6UL_PAD_JTAG_TDI__SAI2_TX_BCLK
MX6UL_PAD_JTAG_TDO__SAI2_TX_SYNC
MX6UL_PAD_JTAG_TRST_B__SAI2_TX_DATA
MX6UL_PAD_JTAG_TCK__SAI2_RX_DATA
MX6UL_PAD_JTAG_TMS__SAI2_MCLK
```
这些内容就是把I.MX6ULL的具体引脚复用成SAI2功能。
**为什么叫“同步”音频接口？**
因为音频数据不是随便发送的，而是必须跟着时钟传输：
```text
BCLK决定每一位数据什么时候传输
LRCK决定当前传输左声道还是右声道
MCLK给WM8960提供基础工作时钟
```
没有正确的时钟，WM8960就无法正确判断音频数据，结果可能是没有声音、噪声或声音失真。
**SAI就是I.MX6ULL内部专门负责音频数据和音频时钟传输的控制器。在WM8960实验中，I2C负责配置WM8960，SAI2通过I2S负责播放和录音数据的传输。**
