# 字符设备

从最小字符设备开始，覆盖设备号分配、`file_operations`、模块加载卸载和用户态测试程序。

代码目录中的 `chrdevbase.c` 是驱动入口，`chrdevbase_test.c` 是配套用户态测试。编译路径和目标内核由各目录下的 Makefile 控制。

对应笔记统一放在本分类的 `notes/` 目录，并在根目录 `NOTE_MAPPING.md` 更新状态。
