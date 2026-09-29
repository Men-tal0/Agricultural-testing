#include "tcpsendder.h"
#include "../../protocol.h"
#include <QDebug>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* ==================== 构造 / 析构 ==================== */
TcpSendder::TcpSendder(const QString &ip, int port, QObject *parent)
    : QThread(parent)
    , m_ip(ip)
    , m_port(port)
{
}

TcpSendder::~TcpSendder()
{
    stop();
    wait();
}

/* ==================== 外部控制 ==================== */
void TcpSendder::stop()
{
    m_running = false;
    m_autoConnect = false;

    if (m_sock >= 0) {
        ::shutdown(m_sock, SHUT_RDWR);
        ::close(m_sock);
        m_sock = -1;
    }
}

void TcpSendder::connectToServer(const QString &ip, int port)
{
    QMutexLocker locker(&m_mutex);
    m_ip = ip;
    m_port = port;
    m_autoConnect = true;
    qDebug() << "connect request:" << ip << ":" << port;
}

void TcpSendder::disconnectFromServer()
{
    m_autoConnect = false;
    if (m_sock >= 0)
    {
        ::shutdown(m_sock, SHUT_RDWR);
        ::close(m_sock);
        m_sock = -1;
    }
    emit disconnected();
}

/* ==================== 数据槽 ==================== */
void TcpSendder::onVideoFrame(QByteArray jpg)
{
    if (jpg.isEmpty()) return;
    QByteArray pkt = packPacket(TYPE_VIDEO, jpg);
    enqueue(pkt);
}

void TcpSendder::onDht11Data(float temp, float humi)
{
    QByteArray body;
    body.append((const char *)&temp, sizeof(temp));
    body.append((const char *)&humi, sizeof(humi));
    enqueue(packPacket(TYPE_DHT11, body));
}

void TcpSendder::onLightData(float lux)
{
    QByteArray body;
    body.append((const char *)&lux, sizeof(lux));
    enqueue(packPacket(TYPE_LIGHT, body));
}

/* ==================== 打包 ==================== */
QByteArray TcpSendder::packPacket(unsigned int type,
                                  const QByteArray &body)
{
    PacketHeader hdr;
    hdr.magic    = PROTOCOL_MAGIC;
    hdr.type     = type;
    hdr.length   = body.size();
    hdr.sequence = m_seq++;

    QByteArray pkt;
    pkt.append((const char *)&hdr, sizeof(hdr));
    pkt.append(body);
    return pkt;
}

/* ==================== 入队 ==================== */
void TcpSendder::enqueue(const QByteArray &packet)
{
    QMutexLocker locker(&m_mutex);

    while (m_queue.size() >= MAX_QUEUE)
    {
        m_queue.dequeue();
    }
    m_queue.enqueue(packet);
}

/* ==================== 连接服务器 ==================== */
int TcpSendder::doConnect()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(m_port);

    if (inet_pton(AF_INET, m_ip.toUtf8().constData(),&addr.sin_addr) <= 0)
    {
        ::close(sock);
        return -1;
    }

    if (::connect(sock, (struct sockaddr *)&addr,sizeof(addr)) < 0)
    {
        ::close(sock);
        return -1;
    }
    return sock;
}

/* ==================== 发送完整 ==================== */
bool TcpSendder::sendAll(const QByteArray &data)
{
    int total = 0;
    int len = data.size();

    while (total < len)
    {
        int n = ::send(m_sock, data.constData() + total,len - total, MSG_NOSIGNAL);
        if (n <= 0) return false;
        total += n;
    }
    return true;
}

/* ==================== 线程主函数 ==================== */
void TcpSendder::run()
{
    m_running = true;

    while (m_running) {
        /* ---- 1. 未连接时尝试连接 ---- */
        if (m_sock < 0)
        {
            if (!m_autoConnect)
            {
                msleep(100);
                continue;
            }

            m_sock = doConnect();
            if (m_sock < 0)
            {
                qDebug() << "connect failed, retry in 3s";
                for (int i = 0; i < 30 && m_running; i++)
                    msleep(100);
                continue;
            }

            qDebug() << "connected to" << m_ip << ":" << m_port;
            emit connected();
        }

        /* ---- 2. 从队列取 ---- */
        QByteArray pkt;
        {
            QMutexLocker locker(&m_mutex);
            if (!m_queue.isEmpty())
                pkt = m_queue.dequeue();
        }

        /* ---- 3. 发送 ---- */
        if (!pkt.isEmpty()) {
            if (!sendAll(pkt)) {
                qDebug() << "send failed, reconnect";
                ::close(m_sock);
                m_sock = -1;
                emit disconnected();
                QMutexLocker locker(&m_mutex);
                m_queue.clear();
            }
        } else {
            msleep(1);
        }
    }

    if (m_sock >= 0)
    {
        ::close(m_sock);
        m_sock = -1;
    }
    qDebug() << "TcpSendder exited";
}
