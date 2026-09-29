#ifndef PROTOCOL_H
#define PROTOCOL_H

#define SERVER_PORT 8888
#define PROTOCOL_MAGIC 0x12345678

/* 数据类型 */
#define TYPE_VIDEO 1
#define TYPE_DHT11 2
#define TYPE_LIGHT 3

/* 帧头：每条消息都以这个开头 */
typedef struct
{
    unsigned int magic;    /* 0x12345678 */
    unsigned int type;     /* 见 TYPE_xxx */
    unsigned int length;   /* 数据长度 */
    unsigned int sequence; /* 序号 */
} PacketHeader;

#endif
