#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include "sensor.h"

static int light_read(float *light)
{
    FILE *fd = fopen("/sys/class/misc/ap3216c/als", "r");
    if(!fd)
    {
        perror("light error");
        return -1;
    }
    int als;
    if (fscanf(fd, "%d", &als) != 1) {
        fclose(fd);
        return -1;
    }
    fclose(fd);
    *light = als * 0.35f;   /* 原始值转 lux，系数看手册 */
    return 0;


}

float light = 0;

int main(int argc, char *argv[])
{
    int msqid = msgget(SENSOR_MSG_KEY,0666 | IPC_CREAT);
    if(msqid < 0)
    {
        perror("msgget");
        return -1;
    }
    printf("light started, msqid=%d\n", msqid);
    while(1)
    {
        if(light_read(&light) == 0)
        {
            SensorMsg msg;
            memset(&msg,0,sizeof(msg));
            msg.mtype = 1;
            msg.sensor_type = sensor_light;
            msg.value1 = light;
            msg.value2 = 0;
            msg.timestamp = time(NULL);
            if(msgsnd(msqid,&msg,sizeof(msg) - sizeof(long),0) < 0)
            {
                perror("msgsnd");
            }
            else
            {
                printf("light=%.1f\n",light);
            }
        }
        sleep(1);
    }
    return 0;
}


