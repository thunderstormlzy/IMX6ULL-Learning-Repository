<!-- Source: Linux内核启动流程（7.23）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux内核启动流程（7.23）

1.vmlinux.lds：链接脚本，来规定内核镜像里各个段怎么排布
2.U-Boot 使用 bootz 启动 Linux：zImage 和 dtb 已经被加载到内存指定地址，bootz 根据参数跳转到 zImage 入口，并把 dtb 地址传递给 Linux 内核。
2.stext：Linux 内核入口（一般用于，检查 CPU、检查启动参数、建立早期页表、准备打开 MMU、跳转到后续 C 语言环境）
3.__mmap_switched：切换到虚拟地址后的过渡点，Linux 会开启 MMU，使用虚拟地址（一般用于，清 BSS、保存处理器信息、准备调用 start_kernel）
4.start_kernel：Linux 内核真正的主初始化函数（一般用于，初始化大量内核核心子系统）
5.rest_init：创建关键内核线程（一般用于，创建 kernel_init 线程、创建 kthreadd 线程、让系统进入调度状态，
其中kernel_init：负责启动用户空间；kthreadd：负责管理内核后台线程
）
6. init 进程：第一个用户空间进程（也就是说把Linux从内核启动变为可用的系统）
