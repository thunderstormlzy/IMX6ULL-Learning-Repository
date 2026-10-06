#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "sys/ioctl.h"
#include "sys/types.h"
#include "sys/stat.h"
#include <poll.h>
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>
#include <fcntl.h>


int main(int argc, char *argv[])
{
    int fd;
    char *filename;
    unsigned short buf[3];
    unsigned short ir,als,ps;
    int ret;

    if(argc!=2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if(fd < 0)
    {
        printf("open file error!\r\n");
        return -1;
    }

    while(1)
    {
        ret = read(fd,buf,sizeof(buf));
        if(ret==0)
        {
            ir = buf[0];
            als = buf[1];
            ps = buf[2];

            printf("ir=%d,als=%d,ps=%d\r\n",ir,als,ps);
            usleep(200000);
        }
    }

    close(fd);
    return 0;
}