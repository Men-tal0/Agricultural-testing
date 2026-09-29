#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QDebug>
#include "tcpreseiver.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_receiver = new tcpreseiver(this);
    connect(m_receiver,&tcpreseiver::videoFrame,this,&Widget::onVideoFrame);
    connect(m_receiver,&tcpreseiver::dht11Data,this,&Widget::onDht11Data);
    connect(m_receiver,&tcpreseiver::lightData,this,&Widget::onLightData);
    connect(m_receiver,&tcpreseiver::clientConnected,this,&Widget::onClientConnected);
    connect(m_receiver,&tcpreseiver::clientDisconnected,this,&Widget::onClientDisconnected);
    connect(m_receiver,&tcpreseiver::errorOccurred,this,[=](QString msg)
    {
        qDebug() << "TcpReceiver error:" << msg;
    });
    m_receiver->start();
}

Widget::~Widget()
{
    delete ui;
    if(m_receiver)
    {
        m_receiver->stop();
        m_receiver->wait();
    }
}

void Widget::onVideoFrame(QImage img)
{
    ui->video->setPixmap(QPixmap::fromImage(img));
}

void Widget::onDht11Data(float temp, float humi)
{
    ui->temp->setText(QString("温度 %1").arg(static_cast <double>(temp),0,'f',1));
    ui->huimi->setText(QString("湿度 %1").arg(static_cast<double>(humi),0,'f',1));
}
void Widget::onLightData(float lux)
{
    ui->light->setText(QString("亮度%1").arg(static_cast<double>(lux),0,'f',1));
}

void Widget::onClientConnected(QString ip)
{
    ui->tcp_static->setText("已连接"+ip);
}

void Widget::onClientDisconnected()
{
    ui->tcp_static->setText("未连接");
}

