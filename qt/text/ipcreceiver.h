#ifndef IPCRECEIVER_H
#define IPCRECEIVER_H


#include <QDebug>
#include <cerrno>
#include <cstring>
#include "../../sensor.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <QThread>
#include <QString>


class Ipcreceiver : public QThread
{
    Q_OBJECT
public:
    Ipcreceiver(QObject *parent = nullptr);
    ~Ipcreceiver();
    void stop();
signals:
    void dht11Data(float temp, float humi);
    void lightData(float lux);
    void errorOccurred(QString msg);
protected:
    void run() override;
private:
    bool m_running = false;

};

#endif // IPCRECEIVER_H
