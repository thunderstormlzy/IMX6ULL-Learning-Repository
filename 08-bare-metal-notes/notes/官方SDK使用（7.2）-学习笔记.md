<!-- Source: 官方SDK使用（7.2）.note | Exported from local Youdao database on 2026-10-06 -->

# 官方SDK使用（7.2）

相比基于STM32库的led驱动，SDK使用的是NXP官方的外设地址文件
MCIMX6Y2.h：定义了芯片上所有外设的寄存器地址和结构体，让开发者可以通过结构体指针来操作寄存器
fsl_iomuxc.h：这个文件专门用于配置芯片的 IOMUXC（IOMUX控制器），它负责管理芯片引脚的多功能复用和电气属性
fsl_common.h：定义了一系列标准的状态枚举_status_groups和_generic_status，用来表示函数执行的结果（如成功、失败、超时等），同时它还有通用数据类型定义的作用，不过在本次操作时，使用了自建的“cc.h”来进行数据类型定义，其中__I / __O / __IO 分别表示只读、只写、读写 volatile 寄存器字段
cc.h
#ifndef __CC_H
#define __CC_H
#define __I     volatile
#define __O     volatile
#define __IO    volatile
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;
typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
#endif
