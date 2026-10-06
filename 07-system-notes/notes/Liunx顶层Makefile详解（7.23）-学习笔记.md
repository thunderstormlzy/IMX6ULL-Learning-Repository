<!-- Source: Liunx顶层Makefile详解（7.23）.note | Exported from local Youdao database on 2026-10-06 -->

# Liunx顶层Makefile详解（7.23）

顶层 Makefile
-> 确定版本、架构、工具链、输出路径
-> 读取 .config
-> 生成自动配置头文件
-> 调用 scripts/ 下的编译脚本
-> 进入 arch/$(ARCH)/Makefile
-> 递归调用各级 Kbuild
-> 编译出 vmlinux、zImage、
modules
、dtb

modules 是 Linux 内核中被配置为 m 的功能集合；
.ko 是这些模块最终编译出来的文件；
它们的作用是让驱动和功能不必全部写死进内核，而是可以在系统运行后按需加载。

xxx_defconfig：默认配置模板
.config：当前真正使用的配置结果
Kconfig：定义配置选项、依赖关系、默认值和菜单结构
include/generated/autoconf.h：给 C 源码使用的 CONFIG 宏
include/config/auto.conf：给 Makefile/Kbuild 使用的 CONFIG 变量
