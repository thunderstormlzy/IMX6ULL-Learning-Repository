<!-- Source: Linux 自带的 LED 灯驱动实验（8.22）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux 自带的 LED 灯驱动实验（8.22）

本次实验通过menuconfig启用Linux自带的LED驱动，因此只需要更改设备树，使compatible与内核中对应 LED 驱动的匹配表一致
同时可以设置“
linux,default-trigger
”属性信息来设置LED灯的默认触发方式
设置led灯的亮度命令为
echo 0 > /sys/class/leds/red/brightness
后面的是 sysfs 控制接口路径
