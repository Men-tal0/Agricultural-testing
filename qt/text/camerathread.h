#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>
#include "camera.h"
#include <QDebug>
#include <QAtomicInt>
#include "../../shm_flag.h"
#include <sys/ioctl.h>
#include <QByteArray>

class CameraThread : public QThread
{
    Q_OBJECT
public:
    CameraThread(QObject *parent = nullptr);
    ~CameraThread();
   void stop();
   void releaseFrame() { m_pending.fetchAndSubOrdered(1);}

signals:
    void frameReady(QImage img);
    void errorOccurred(QString msg);
    void frameForUpload(QByteArray jpg);  //给tcpsendder发送原格式
protected:
    void run() override;

private:
    bool m_running = false;
     QAtomicInt m_pending{0};  //未处理帧计数
     ShmFlag *m_flag = nullptr;
};

#endif // CAMERATHREAD_H
