<!-- Source: Linux阻塞和非阻塞IO（8.17）.note | Exported from local Youdao database on 2026-10-06 -->

# Linux阻塞和非阻塞IO（8.17）

当设备暂时没有数据可读、不能写入或事件尚未发生时，阻塞 IO 会让当前进程进入睡眠，直到条件满足后被唤醒。对于非阻塞 IO， 在条件不满足时立即返回，通常返回 -EAGAIN；应用程序可以自行重试，也可以使用 poll/select/epoll 等待设备就绪。
主要作用是管理CPU资源 + 控制程序流程，可以使用top命令查看cpu占用。
fd = open("/dev/xxx_dev", O_RDWR); /* 阻塞方式打开 */
fd = open("/dev/xxx_dev", O_RDWR | O_NONBLOCK);/* 非阻塞方式打开 */

阻塞IO相关操作：等待队列
等待队列头：当设备不可访问的时候就需要将进程对应的等待队列项添加到等待队列头中，只有添加到等待队列头中以后进程才能进入休眠态。
等待队列项：
每个访问设备的进程都是一个等待队列项
具体流程：
设置当前进程状态
-> 将进程加入等待队列
-> 检查等待条件
-> 条件不满足则 schedule
-> 被唤醒后重新检查条件
-> 条件满足则移出等待队列并继续执行
void init_waitqueue_head(wait_queue_head_t *q)
参数 q 就是要初始化的等待队列头。也可以使用宏 DECLARE_WAIT_QUEUE_HEAD 来一次性完成等待队列头的定义的初始
化。
DECLARE_WAITQUEUE(name, tsk)
使用宏 DECLARE_WAITQUEUE 定义并初始化一个等待队列项，name 就是等待队列项的名字，tsk 表示这个等待队列项属于哪个任务(进程)，一般设置为 current，在 Linux 内核中 current 相当于一个全局变量，表示当前进程。
void add_wait_queue(wait_queue_head_t *q, wait_queue_t *wait)
等待队列项添加 API 函数，q：等待队列项要加入的等待队列头，wait：要加入的等待队列项，返回值：无
void remove_wait_queue(wait_queue_head_t *q, wait_queue_t *wait)
等待队列项移除 API 函数，q：要删除的等待队列项所处的等待队列头，wait：要删除的等待队列项，返回值：无
void wake_up(wait_queue_head_t *q)
void wake_up_interruptible(wait_queue_head_t *q)
当设备可以使用的时候就要唤醒进入休眠态的进程，参数 q 就是要唤醒的等待队列头，这两个函数会将这个等待队列头中的所有进程都唤醒。wake_up 函数可以唤醒处于 TASK_INTERRUPTIBLE 和 TASK_UNINTERRUPTIBLE 状态的进程，而 wake_up_interruptible 函数只能唤醒处于 TASK_INTERRUPTIBLE 状态的进程。
schedule();是当前进程主动暂停自己，让 Linux 调度器更换其他进程运行；等自己被唤醒并再次被选中后，从 schedule() 后面继续之前的进程。
除了主动唤醒以外，也可以设置等待队列等待某个事件，当这个事件满足以后就自动唤醒等待队列中的进程

非阻塞IO相关操作：轮询
用户应用程序以非阻塞的方式访问设备，设备驱动程序就要提供非阻塞的处理方式，也就是轮询。poll、epoll 和 select 可以用于处理轮询，应用程序通过 
select、epoll 或 poll
 函数来查询设备是否可以操作，如果可以操作的话就从设备读取或者向设备写入数据。
具体流程：
应用程序使用 O_NONBLOCK 打开设备
-> 调用 read/write
-> 驱动检查设备状态
-> 数据未准备好或设备暂时不可写
-> 立即返回 -EAGAIN
-> 应用程序自行处理
int select(int nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, struct timeval *timeout)
nfds：所要监视的这三类文件描述集合中，最大文件描述符加 1。
readfds、writefds 和 exceptfds：这三个指针指向描述符集合，readfds 用于监视指定描述符集的读变化，也就是监视这些文件是否可以读取，只要这些集合里面有一个文件可以读取那么 select 就会返回一个大于 0 的值表示文件可以读取。如果没有文件可以读取，那么就会根据 timeout 参数来判断是否超时。可以将 readfs 设置为 NULL，表示不关心任何文件的读变化。writefds 和 readfs 类似，只是 writefs 用于监视这些文件是否可以进行写操作。exceptfds 用于监视这些文件的异常。
返回值：0，表示的话就表示超时发生，但是没有任何文件描述符可以进行操作；-1，发生错误；其他值，可以进行操作的文件描述符个数。
fd_set相关操作：
void FD_ZERO(fd_set *set)
void FD_SET(int fd, fd_set *set)
void FD_CLR(int fd, fd_set *set)
int FD_ISSET(int fd, fd_set *set)
FD_ZERO 用于将 fd_set 变量的所有位都清零，FD_SET 用于将 fd_set 变量的某个位置 1，也就是向 fd_set 添加一个文件描述符，参数 fd 就是要加入的文件描述符。FD_CLR 用于将 fd_set 变量的某个位清零，也就是将一个文件描述符从 fd_set 中删除，参数 fd 就是要删除的文件描述符。FD_ISSET 用于测试一个文件是否属于某个集合，参数 fd 就是要判断的文件描述符。
int poll(struct pollfd *fds,  nfds_t nfds,  int timeout)
在单个线程中，select 函数能够监视的文件描述符数量有最大的限制，一般为 1024，可以修改内核将监视的文件描述符数量改大，但是这样会降低效率！这个时候就可以使用 poll 函数，poll 函数本质上和 select 没有太大的差别，但是 poll 函数没有最大文件描述符限制
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
fds：要监视的文件描述符集合以及要监视的事件,为一个数组，数组元素都是结构体 pollfd类型的
struct pollfd {
int fd; /* 文件描述符 */
short events; /* 请求的事件 */
 short revents; /* 返回的事件 */
};
fd 是要监视的文件描述符，如果 fd 无效的话那么 events 监视事件也就无效，并且 revents
返回 0。events 是要监视的事件，可监视的事件类型如下所示：
POLLIN 有数据可以读取。
POLLPRI 有紧急的数据需要读取。
POLLOUT 可以写数据。
POLLERR 指定的文件描述符发生错误。
POLLHUP 指定的文件描述符挂起。
POLLNVAL 无效的请求。
POLLRDNORM 等同于 POLLIN
revents 是返回参数，也就是返回的事件，由 Linux 内核设置具体的返回事件。
nfds：poll 函数要监视的文件描述符数量。
timeout：超时时间，单位为 ms。
返回值：返回 revents 域中不为 0 的 pollfd 结构体个数，也就是发生事件或错误的文件描述
符数量；0，超时；-1，发生错误，并且设置 errno 为错误类型。
int epoll_ctl
(int epfd, 
 int op, 
 int fd,
 struct epoll_event *event)
函数参数和返回值含义如下：
epfd：要操作的 epoll 句柄，也就是使用 epoll_create 函数创建的 epoll 句柄。
op：表示要对 epfd(epoll 句柄)进行的操作，可以设置为：
EPOLL_CTL_ADD 向 epfd 添加文件参数 fd 表示的描述符。
EPOLL_CTL_MOD 修改参数 fd 的 event 事件。
EPOLL_CTL_DEL 从 epfd 中删除 fd 描述符。
fd：要监视的文件描述符。
event：要监视的事件类型
传统的 selcet 和 poll 函数都会随着所监听的 fd 数量的增加，出现效率低下的问题，而且 poll 函数每次必须遍历所有的描述符来检查就绪的描述符，这个过程很浪费时间。为此，epoll 应运而生，epoll 就是为处理大并发而准备的，一般常常在网络编程中使用 epoll 函数。
