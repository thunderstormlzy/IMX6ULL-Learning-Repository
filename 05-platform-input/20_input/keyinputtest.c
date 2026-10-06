#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/input.h>
#include <poll.h>
#include <sys/select.h>
#include <signal.h>

static struct input_event inputevent;


int main(int argc, char *argv[])
{
    int fd;
    int err =0 ;
    char*filename;

    filename = argv[1];
    if(argc!=2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    fd = open(filename, O_RDWR);
    if(fd<0)
    {
        printf("open %s failed\r\n", filename);
        return -1;
    }

    while(1)
    {
        err = read(fd, &inputevent, sizeof(struct input_event));
        if(err>0)
        {
            switch(inputevent.type)
            {
                case EV_KEY:
                    if(inputevent.code<BTN_MISC)
                    {    
                        printf("key %d %s\r\n", inputevent.code, inputevent.value?"press":"release");
                    }
                    else
                    {
                        printf("button %d %s\r\n", inputevent.code, inputevent.value?"press":"release");
                    }
                    break;
                case EV_REL:
                break;
                case EV_ABS:
                break;
                case EV_SW:
                break;
                case EV_MSC:
                break;
            }
        }
        else
        {
            printf("read error\r\n");
        }
    }
    return 0;
}
