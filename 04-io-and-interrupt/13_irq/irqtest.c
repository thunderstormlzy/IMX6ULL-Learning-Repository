#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"

int main(int argc,char *argv[])
{
    int fd;
    int ret = 0;
    char *filename;
    unsigned char data;

    if(argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if(fd < 0)
    {
        printf("open %s failed\r\n", filename);
        return -1;
    }

    while(1)
    {
        ret = read(fd, &data, sizeof(data));
        if(ret < 0)
        {
        }
        else
        {
            printf("keyvalue=%d\r\n", data);
        }
    }
    close(fd);
    return 0;
}