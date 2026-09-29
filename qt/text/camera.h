#ifndef CAMERA_H
#define CAMERA_H

#ifdef __cplusplus
extern "C"
{
#endif

    int camera_init(const char *device);
    int camera_release_frame(void);
    int camera_get_frame(unsigned char **data, size_t *len);
    void camera_stop(void);
    int camera_get_fd(void);
    int camera_requeue_buffers(void);
#ifdef __cplusplus
}
#endif

#endif
