#include "tcpreseiver.h"
#include "../../protocol.h"

#include <QDebug>
#include <QByteArray>

#include <opencv2/opencv.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

tcpreseiver::tcpreseiver(QObject *parent) : QThread(parent)
{
}

tcpreseiver::~tcpreseiver()
{
    stop();
    wait();
}

void tcpreseiver::stop()
{
    m_running = false;

    if (m_listen_fd >= 0) {
        ::close(m_listen_fd);
        m_listen_fd = -1;
    }
}

void tcpreseiver::run()
{
    /* ---------- 1. socket ---------- */
    m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen_fd < 0)
    {
        emit errorOccurred(QString("socket: %1").arg(strerror(errno)));
        return;
    }

    int opt = 1;
    setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    /* ---------- 2. bind ---------- */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));      /* ★ 必须 memset */
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listen_fd, (struct sockaddr *)&addr,sizeof(addr)) < 0)
    {
        emit errorOccurred(QString("bind: %1").arg(strerror(errno)));
        ::close(m_listen_fd);
        m_listen_fd = -1;
        return;
    }

    /* ---------- 3. listen ---------- */
    if (listen(m_listen_fd, 5) < 0)
    {
        emit errorOccurred(QString("listen: %1").arg(strerror(errno)));
        ::close(m_listen_fd);
        m_listen_fd = -1;
        return;
    }

    qDebug() << "=== Server listening on port" << SERVER_PORT << "===";
    m_running = true;

    /* ---------- 4. accept 循环 ---------- */
    while (m_running)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int connfd = accept(m_listen_fd,
                            (struct sockaddr *)&client_addr,
                            &client_len);
        if (connfd < 0)
        {
            if (!m_running) break;
            continue;
        }

        QString client_ip = inet_ntoa(client_addr.sin_addr);
        qDebug() << "Client connected:" << client_ip;
        emit clientConnected(client_ip);

        /* ---------- 5. recv 循环 ---------- */
        while (m_running)
        {
            PacketHeader hdr;

            /* ★ 收帧头：3 个参数一个不能少 */
            int n = recv(connfd, &hdr, sizeof(hdr), MSG_WAITALL);
            if (n <= 0) {
                qDebug() << "client disconnected";
                break;
            }

            if (hdr.magic != PROTOCOL_MAGIC)
            {
                qDebug() << "bad magic: 0x" << QString::number(hdr.magic, 16);
                break;
            }

            /* ★ case 里定义变量要加花括号 */
            switch (hdr.type)
            {
                case TYPE_VIDEO:
                {
                    QByteArray jpg(hdr.length, 0);
                    if (recv(connfd, jpg.data(), hdr.length,MSG_WAITALL) <= 0)
                        break;

                    cv::Mat raw(1, jpg.size(), CV_8UC1,
                            (void *)jpg.constData());
                    cv::Mat bgr = cv::imdecode(raw, cv::IMREAD_COLOR);
                    if (!bgr.empty())
                    {
                        cv::Mat rgb;
                        cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
                        QImage img(rgb.data, rgb.cols, rgb.rows,
                                   rgb.step, QImage::Format_RGB888);
                        emit videoFrame(img.copy());
                    }
                    break;
                 }
            case TYPE_DHT11:
            {
                float data[2];
                /* ★ 去掉多余的逗号 */
                if (recv(connfd, data, sizeof(data),MSG_WAITALL) <= 0)
                    break;
                emit dht11Data(data[0], data[1]);
                break;
            }
            case TYPE_LIGHT:
            {
                float lux;
                /* ★ 同样去掉多余逗号 */
                if (recv(connfd, &lux, sizeof(lux),MSG_WAITALL) <= 0)
                    break;
                emit lightData(lux);
                break;
            }
            default:
                qDebug() << "unknown type:" << hdr.type;
                break;
            }
        }

        /* ★ close(connfd) 在内层循环外 */
        ::close(connfd);
        emit clientDisconnected();
        qDebug() << "client closed";
    }

    /* ★ 清理 listen fd，在外层 while 之外 */
    if (m_listen_fd >= 0) {
        ::close(m_listen_fd);
        m_listen_fd = -1;
    }
    qDebug() << "TcpReceiver exited";
}
