#ifndef SHM_FLAG_H
#define SHM_FLAG_H

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define SHM_FLAG_PATH "/dev/shm/dht11_camera_flag"

typedef struct
{
    volatile int dht11_reading; /* 1 = DHT11 正在读，0 = 空闲 */
} ShmFlag;

static inline ShmFlag *shm_flag_open(void)
{
    int fd = open(SHM_FLAG_PATH, O_RDWR | O_CREAT,0666);
    if (fd < 0)
    {
        perror("open shm_flag");
        return NULL;
    }
    if (ftruncate(fd, sizeof(ShmFlag)) < 0)
    {
        perror("ftruncate");
        return NULL;
    }
    ShmFlag *p = (ShmFlag *)mmap(NULL, sizeof(ShmFlag), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (p == MAP_FAILED)
    {
        perror("mmap");
        return NULL;
    }
    return p;
}

#endif
