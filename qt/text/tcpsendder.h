#ifndef TCPSENDDER_H
#define TCPSENDDER_H

#include <QThread>
#include <QByteArray>
#include <QQueue>
#include <QMutex>
#include <QString>

class TcpSendder : public QThread    /* ★ 类名首字母大写，和 .cpp 统一 */
{
    Q_OBJECT

public:
    explicit TcpSendder(const QString &ip, int port,
                        QObject *parent = nullptr);
    ~TcpSendder();

    /* 外部控制 */
    void stop();
    void connectToServer(const QString &ip, int port);
    void disconnectFromServer();
    bool isConnected() const { return m_sock >= 0; }

public slots:
    void onVideoFrame(QByteArray jpg);
    void onDht11Data(float temp, float humi);
    void onLightData(float lux);

signals:
    void connected();
    void disconnected();
    void errorOccurred(QString msg);

protected:
    void run() override;

private:
    /* 内部工具 */
    void enqueue(const QByteArray &packet);
    QByteArray packPacket(unsigned int type, const QByteArray &body);
    bool sendAll(const QByteArray &data);
    int  doConnect();

    /* 成员 */
    QString m_ip;
    int     m_port;

    int  m_sock = -1;              /* -1 = 未连接 */
    bool m_running = false;
    bool m_autoConnect = false;    /* 按钮点后才开始连接 */

    QMutex m_mutex;
    QQueue<QByteArray> m_queue;
    quint32 m_seq = 0;

    static const int MAX_QUEUE = 30;
};

#endif
