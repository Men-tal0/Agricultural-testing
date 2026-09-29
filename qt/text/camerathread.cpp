#include "camerathread.h"

CameraThread::CameraThread(QObject *parent) : QThread (parent)
{

}

CameraThread::~CameraThread()
{
    stop();
    wait();
}

void CameraThread::stop()
{
    m_running = false;
}

void CameraThread::run()
{
    //static int frame_cnt = 0;
    //qDebug() << "emit frame" << frame_cnt++;

    if(camera_init("/dev/video2") < 0)
    {
        emit errorOccurred("camera_init err");
        return;
    }
    //指向共享内存
    m_flag = shm_flag_open();
    if(!m_flag)
    {
        emit errorOccurred("shm_flag_open failed");
        camera_stop();
        return;
    }

    m_running = true;
    enum v4l2_buf_type  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while(m_running)
    {
        if(m_flag->dht11_reading == 1)
        {
            ioctl(camera_get_fd(), VIDIOC_STREAMOFF, &type);
            while(m_flag->dht11_reading == 1 && m_running)
            {
                msleep(1);
            }
            camera_requeue_buffers();
            ioctl(camera_get_fd(), VIDIOC_STREAMON, &type);
            continue;
        }
        unsigned char *data = nullptr;
        size_t len = 0;
        if(camera_get_frame(&data,&len) < 0)
        {
            if(!m_running) break;
            emit errorOccurred("camera_get_frame err");
            break;
        }
        if(m_pending.loadAcquire() < 1)
        {
            //发送数据
            m_pending.fetchAndAddOrdered(1);
            QByteArray jpg((const char *)data, (int)len);
            emit frameForUpload(jpg);

            //可以减少数据拷贝，直接mjpg是一维数组，所以直接1行len列，data就是这个数组的地址，将raw指向mmap这块内存空间
            cv::Mat raw(1,len,CV_8UC1,data);
            //将这块内存数据进行解码操作，opencv默认解码完数据三bgr格式
            cv::Mat bgr = cv::imdecode(raw,cv::IMREAD_COLOR);
            if(!bgr.empty())
            {
                cv::Mat rgb;
                cv::cvtColor(bgr,rgb,cv::COLOR_BGR2RGB);
                QImage img(rgb.data,rgb.cols,rgb.rows,rgb.step,QImage::Format_RGB888);
                emit frameReady(img.copy());
            }
        }
        camera_release_frame();
        msleep(5);
    }
    camera_stop();
}
