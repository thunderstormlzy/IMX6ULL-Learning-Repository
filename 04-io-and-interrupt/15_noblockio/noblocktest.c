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

int main(int argc, char *argv[])
{
    int fd;
    int ret=0;
    char *filename;
    struct pollfd pfd;
    fd_set rfds;
    struct timeval timeout;
    unsigned char data;

    if(argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];
    fd = open(filename, O_RDONLY|O_NONBLOCK);

    while(1)
    {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;
        ret = select(fd+1, &rfds, NULL, NULL, &timeout);
        if(ret < 0)
        {
            printf("select error!\r\n");
            return -1;
        }
        else if(ret == 0)
        {
            printf("timeout!\r\n");
            continue;
        }
        else
        {
            if(FD_ISSET(fd, &rfds))
            {
                ret = read(fd, &data, sizeof(data));
                if(ret < 0){

                }
                else{
                    printf("keyvalue=%d\r\n", data);
                }
            }
        }
    }
        close(fd);
        return ret;
}
