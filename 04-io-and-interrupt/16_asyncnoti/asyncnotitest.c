#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "poll.h"
#include "sys/select.h"
#include "sys/time.h"
#include "linux/ioctl.h"
#include "signal.h"

static int fd = 0;

static void sigio_signal(int sig)
{
    int ret=0;
    unsigned int keyvalue=0;

    ret = read(fd, &keyvalue, sizeof(keyvalue));
    if(ret < 0){
        printf("read error!\r\n");
    }
    else{
        printf("keyvalue=%d\r\n", keyvalue);
    }

}

int main(int argc, char *argv[])
{
    int flags = 0;
    int ret=0;
    char *filename;


    if(argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];
    fd = open(filename, O_RDWR);
    if(fd < 0)
    {
        printf("open error!\r\n");
        return -1;
    }

    signal(SIGIO, sigio_signal);

    fcntl(fd, F_SETOWN, getpid());

    flags = fcntl(fd, F_GETFD);
    fcntl(fd, F_SETFL, flags | FASYNC);


    while(1)
    {
        sleep(2);
    }
        close(fd);
        return ret;
}
