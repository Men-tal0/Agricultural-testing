#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "camerathread.h"
#include "ipcreceiver.h"
#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <unistd.h>       /* fork, execl, _exit */
#include <signal.h>       /* kill, SIGTERM */
#include <sys/types.h>    /* pid_t */
#include "tcpsendder.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void onFrameReady(QImage img);
    void onErrorOccurred(QString msg);
    void onDht11Data(float temp, float humi);
    void onLightData(float lux);
    void on_connect_clicked();
private:
    void startSensorProcesses();
    void killSensorProcesses();
    Ui::Widget *ui;
    CameraThread *m_camera;
    Ipcreceiver  *m_ipc;
    QList<pid_t>  m_childPids;
    TcpSendder *m_sender;
};
#endif // WIDGET_H
