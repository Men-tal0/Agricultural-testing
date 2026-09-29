#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include "sensor.h"
#include "shm_flag.h"

static int dht11_read(float *temp, float *huimi)
{
    FILE *fd = fopen("/sys/class/misc/dht11/value", "r");
    if (!fd)
    {
        perror("dht11 read err");
        return -1;
    }
    int n = fscanf(fd, "%f,%f", huimi, temp);
    if (n != 2)
    {
        printf("dht11 fscanf failed, n=%d\n", n); // ★ 打印返回值
        return -1;
    }

    fclose(fd);
    return 0;
}

float temp = 0, huimi = 0;

int main(void)
{
    // 共享内存
    ShmFlag *flag = shm_flag_open();
    if (!flag)
    {
        perror("shm_flag_open");
        return -1;
    }
    flag->dht11_reading = 0;
    // 消息队列
    int msqid = msgget(SENSOR_MSG_KEY, 0666 | IPC_CREAT);
    if (msqid < 0)
    {
        perror("msqid error");
        return -1;
    }
    printf("dht11_proc started, msqid=%d\n", msqid);

    while (1)
    {
        // 关闭摄像头
        flag->dht11_reading = 1;
        usleep(5000);

        if (dht11_read(&temp, &huimi) == 0)
        {
            // 恢复摄像头
            flag->dht11_reading = 0;
            SensorMsg msg;
            memset(&msg, 0, sizeof(msg));
            msg.mtype = 1;
            msg.sensor_type = sensor_dht11;
            msg.value1 = temp;
            msg.value2 = huimi;
            msg.timestamp = time(NULL);
            if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), IPC_NOWAIT) < 0)
            {
                perror("msgsnd");
            }
            else
            {
                printf("DHT11: temp=%.1f, humi=%.1f\n", temp, huimi);
            }
        }

        sleep(2);
    }
    return 0;
}
