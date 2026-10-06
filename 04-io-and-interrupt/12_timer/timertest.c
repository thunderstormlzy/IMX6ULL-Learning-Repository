#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define TIMER_IOC 'T'
#define OPEN_CMD (_IO(TIMER_IOC, 0x2))
#define CLOSE_CMD (_IO(TIMER_IOC, 0x1))
#define SETPERIOD_CMD (_IO(TIMER_IOC, 0x3))

int main(int argc, char *argv[])
{
    int fd;
    int ret;
    char *filename;
    unsigned int cmd;
    unsigned int arg;
    unsigned char str[100];

    if(argc < 2) {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];
    fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("open timer failed!\r\n");
        return -1;
    }
while(1) {
    printf("input cmd:\r\n");
    ret =scanf("%d", &cmd);
    if (ret != 1) {
        fgets(str, 100, stdin);
    }

    if(cmd==1)
        {
            cmd = OPEN_CMD;
        }
    else if(cmd==2)
        {
            cmd = CLOSE_CMD;
        }
    else if(cmd==3)
        {
            cmd = SETPERIOD_CMD;
            printf("input timer period:\r\n");
            ret = scanf("%d", &arg);
            if (ret != 1) {
                fgets(str, 100, stdin);
            }
        }
        else
        {
            printf("invalid cmd!\r\n");
        }
    
        ioctl(fd, cmd, arg);
    }
    close(fd);
}