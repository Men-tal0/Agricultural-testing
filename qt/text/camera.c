#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <error.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include "camera.h"

#define cam_w 320
#define cam_h 240
#define readbuf_count 4

int fd;                                   // 摄像头文件描述符
unsigned char *kernal_buf[readbuf_count]; // 存放mmap(内存映射)的地址

struct v4l2_capability cap;         // 摄像头能力结构体
struct v4l2_format vfmt;            // 设置摄像头格式结构体
struct v4l2_requestbuffers readbuf; // 设置缓冲区结构体
struct v4l2_buffer mmap_buf;        // 设置mmap缓冲区结构体
struct v4l2_buffer read_buf;        // 循环读取的结构体

//struct v4l2_streamparm parm;

int camera_init(const char *device)
{
    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        perror("open device error");
        return -1;
    }
    // 查看支持
    // struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        perror("query capability error");
        return -1;
    }
    // 查看是否支持视频采集
    if (!(cap.device_caps & V4L2_CAP_VIDEO_CAPTURE))
    {
        perror("not support capture");
        return -1;
    }
    // 查看是否支持mmap
    if (!(cap.device_caps & V4L2_CAP_STREAMING))
    {
        perror("not support streaming");
        return -1;
    }

    // 设置摄像头格式
    // struct v4l2_format vfmt;
    vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vfmt.fmt.pix.width = cam_w;
    vfmt.fmt.pix.height = cam_h;
    vfmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    if (ioctl(fd, VIDIOC_S_FMT, &vfmt) < 0)
    {
        perror("set format error");
        return -1;
    }

    //struct v4l2_streamparm parm;   帧率
//    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//    parm.parm.capture.timeperframe.numerator = 1;
//    parm.parm.capture.timeperframe.denominator = 15;
//    ioctl(fd, VIDIOC_S_PARM, &parm);

    // 设置缓冲区数量
    // struct v4l2_requestbuffers readbuf;
    readbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    readbuf.count = readbuf_count;
    readbuf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &readbuf) < 0)
    {
        perror("request buffer error");
        return -1;
    }

    // 映射缓冲区
    int i = 0;
    // struct v4l2_buffer mmap_buf;
    mmap_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for (i = 0; i < readbuf_count; i++)
    {
        mmap_buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &mmap_buf) < 0) // 查询缓冲区
        {
            perror("query buffer error");
            return -1;
        }
        kernal_buf[i] = mmap(NULL, mmap_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mmap_buf.m.offset);
        if (ioctl(fd, VIDIOC_QBUF, &mmap_buf) < 0) // 缓冲区的所有权交还给驱动
        {
            perror("queue buffer error");
        }
    }

    // 开启采集
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0)
    {
        perror("stream on error");
        return -1;
    }
    return 0;
}

int camera_get_frame(unsigned char **data, size_t *len)
{
    // struct v4l2_buffer read_buf;
    read_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    read_buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_DQBUF, &read_buf) < 0)
    {
        perror("dequeue buffer error");
        return -1;
    }

    // 用于给qt使用的函数，获取数据
    *data = kernal_buf[read_buf.index];
    *len = read_buf.bytesused;
    return 0;
}

int camera_release_frame(void)
{
    if (ioctl(fd, VIDIOC_QBUF, &read_buf) < 0)
    {
        perror("queue buffer error");
        return -1;
    }
    return 0;
}

void camera_stop(void)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
    {
        perror("stream off error");
        return;
    }
    for (int i = 0; i < readbuf_count; i++)
    {
        munmap(kernal_buf[i], mmap_buf.length);
    }
    close(fd);
}

/* ★ 重新把所有缓冲区入队 */
int camera_requeue_buffers(void)
{
    for (int i = 0; i < readbuf_count; i++)
    {
        struct v4l2_buffer buf;
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;

        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0)
        {
            perror("QBUF");
            return -1;
        }
    }
    return 0;
}

int camera_get_fd(void)
{
    return fd;
}

