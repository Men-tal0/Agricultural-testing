#ifndef SENSOR_H
#define SENSOR_H

#define SENSOR_MSG_KEY  0x1234
typedef enum
{
    sensor_dht11 = 1,
    sensor_light = 2,
} sensortype;

/* 消息结构体
 * 注意：第一个字段必须是 long mtype,System V 消息队列的硬性要求
 */
typedef struct {
    long  mtype;         /* 消息类型，固定填 1 */
    int   sensor_type;   /* 哪种传感器，见 SensorType */
    float value1;        /* 主值：温度 / 光照 / 气体 */
    float value2;        /* 副值：湿度 / 备用 */
    long  timestamp;     /* 时间戳，秒 */
} SensorMsg;

#endif

