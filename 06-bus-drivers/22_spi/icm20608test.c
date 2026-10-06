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
    signed int data[7];
    unsigned char buf[14];
    signed int gyro_x_adc, gyro_y_adc, gyro_z_adc;
    signed int accel_x_adc, accel_y_adc, accel_z_adc;
    signed int temp_adc;

    float gyro_x, gyro_y, gyro_z;
    float accel_x, accel_y, accel_z;
    float temp;

    int ret =0;

    if(argc != 2){
        printf("Usage: %s <filename>\r\n", argv[0]);
        return -1;
    }

    filename=argv[1];
    fd = open(filename, O_RDWR);
    if(fd < 0){
        printf("open %s err\r\n", filename);
        return -1;
    }

    while(1){
        ret=read(fd, data, sizeof(data));
        if(ret==sizeof(data)){
            accel_x_adc=data[0];
            accel_y_adc=data[1];
            accel_z_adc=data[2];
            temp_adc=data[3];
            gyro_x_adc=data[4];
            gyro_y_adc=data[5];
            gyro_z_adc=data[6];

            gyro_x=(float)(gyro_x_adc)/16.4;
            gyro_y=(float)(gyro_y_adc)/16.4;
            gyro_z=(float)(gyro_z_adc)/16.4;

            accel_x=(float)(accel_x_adc)/2048.0;
            accel_y=(float)(accel_y_adc)/2048.0;
            accel_z=(float)(accel_z_adc)/2048.0;

            temp=((float)(temp_adc)-25)/326.8+25;

            printf("\r\n 原始值：\r\n");
            printf("gyro: x=%dg y=%dg z=%dg\r\n", gyro_x_adc, gyro_y_adc, gyro_z_adc);
            printf("accel: x=%dg y=%dg z=%dg\r\n", accel_x_adc, accel_y_adc, accel_z_adc);
            printf("temp: %d°C\r\n", temp_adc);


            printf("实际值：\r\n");
            printf("gyro: x=%.2fg y=%.2fg z=%.2fg\r\n", gyro_x, gyro_y, gyro_z);
            printf("accel: x=%.2fg y=%.2fg z=%.2fg\r\n", accel_x, accel_y, accel_z);
            printf("temp: %.2f°C\r\n", temp);
        }
        usleep(100000);
    }
    close(fd);
    return 0;
}