#include"stdio.h"
#include"unistd.h"
#include"stdlib.h"
#include"string.h"
#include"sys/types.h"
#include"sys/stat.h"
#include"fcntl.h"

#define LED_ON 1
#define LED_OFF 0

int main(int argc, char *argv[])
{
    int fd;
    unsigned char buf[1];
    int ret;
    char *filename;
    unsigned int cnt=0;
    if(argc != 3) {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if(fd < 0) {
        printf("Open %s Failed!\r\n", filename);
        return -1;
    }

    buf[0] = atoi(argv[2]);

    ret = write(fd, buf, sizeof(buf));
    if(ret < 0) {
        printf("Write Failed!\r\n");
        close(fd);
        return -1;
    }

    while(1) {
        sleep(5);
        cnt++;
        printf("App running times:%d\r\n", cnt);
        if(cnt >= 5) break;
    }

    printf("App exit!\r\n");

    ret = close(fd);
    if(ret < 0) {
        printf("Close Failed!\r\n");
        return -1;
    }
    return 0;
}