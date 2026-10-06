#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define KEY1VALUE 1
#define KEY0VALUE 0

int main(int argc, char *argv[])
{
    int fd;
    unsigned char keyvalue;
    int ret;
    char *filename;


    if (argc != 2) {
        printf("Error Usage\r\n");
        return -1;
    }

    filename = argv[1];
    fd = open(filename, O_RDONLY);

    if (fd < 0) {
        printf("open key failed!\r\n");
        return -1;
    }

    while(1){
        ret = read(fd, &keyvalue, sizeof(keyvalue));
        
        if (ret < 0) {
            printf("read key failed!\r\n");
            return -1;
        }

        if(keyvalue == KEY1VALUE) {
            printf("key down! key value: %d\r\n", keyvalue);
        }
    }

    ret = close(fd);
    if (ret < 0) {
        printf("close key failed!\r\n");
        return -1;
    }
    return 0;
}