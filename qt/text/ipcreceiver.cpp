#include "ipcreceiver.h"
#include <unistd.h>

Ipcreceiver::Ipcreceiver(QObject *parent) :  QThread(parent)
{

}

Ipcreceiver::~Ipcreceiver()
{
    stop();
    wait();
}

void Ipcreceiver::stop()
{
    m_running = false;
}


void Ipcreceiver::run()
{
    int msqid = msgget(SENSOR_MSG_KEY,0666|IPC_CREAT);
    if(msqid < 0)
    {
        emit errorOccurred(QString("msgget failed").arg(strerror(errno)));
        return;
    }
    m_running = true;
    while(m_running)
    {
        SensorMsg msg;
        memset(&msg, 0, sizeof(msg));
        ssize_t n = msgrcv(msqid,&msg,sizeof(msg) - sizeof(long),0,IPC_NOWAIT);
        if(n<0)
        {
            if(errno == ENOMSG)
            {
                msleep(10);
                continue;
            }
            emit  errorOccurred(QString("msgrcv failed").arg(strerror(errno)));
            break;
        }
        switch(msg.sensor_type)
        {
            case sensor_dht11:
                emit dht11Data(msg.value1,msg.value2);
            break;
            case sensor_light:
                emit lightData(msg.value1);
            break;
            default:
                qDebug() << "unknown sensor_type:" << msg.sensor_type;
                break;
        }
    }
}
