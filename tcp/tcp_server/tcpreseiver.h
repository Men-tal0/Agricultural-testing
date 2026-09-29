#ifndef TCPRESEIVER_H
#define TCPRESEIVER_H

#include <QThread>
#include <QImage>
#include <QString>

class tcpreseiver : public QThread
{
    Q_OBJECT
public:
    tcpreseiver(QObject *parent = nullptr);
    ~tcpreseiver();
    void stop();

signals:
    /* 收到一帧视频 */
    void videoFrame(QImage img);

    /* 收到传感器数据 */
    void dht11Data(float temp, float humi);
    void lightData(float lux);

    /* 客户端连接状态 */
    void clientConnected(QString ip);
    void clientDisconnected();

    /* 出错 */
    void errorOccurred(QString msg);
protected:
    void run() override;

private:
    bool m_running = false;
    int  m_listen_fd = -1;

};

#endif // TCPRESEIVER_H
