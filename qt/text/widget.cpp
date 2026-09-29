#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //tcp
    m_sender = new TcpSendder("192.168.1.180", 8888, this);
    connect(m_sender, &TcpSendder::connected, this, [=](){
        ui->information->setText("已连接");
        ui->information->setStyleSheet("color: green;");
        ui->connect->setText("断开");
    });
    connect(m_sender, &TcpSendder::disconnected, this, [=](){
         ui->information->setText("已断开");
         ui->information->setStyleSheet("color: gray;");
         ui->connect->setText("连接");
     });
     m_sender->start();
    //camera
    m_camera = new CameraThread(this);
    connect(m_camera,&CameraThread::frameReady,this,&Widget::onFrameReady);
    connect(m_camera,&CameraThread::errorOccurred,this,&Widget::onErrorOccurred);
    m_camera->start();
    //ipc
    m_ipc = new Ipcreceiver(this);
    connect(m_ipc,&Ipcreceiver::dht11Data,this,&Widget::onDht11Data);
    connect(m_ipc,&Ipcreceiver::lightData,this,&Widget::onLightData);
    connect(m_ipc,&Ipcreceiver::errorOccurred,this,[=](QString msg){
        qDebug() << "IpcReceiver error:" << msg;
    });
    //发送给服务器的数据
    connect(m_camera, &CameraThread::frameForUpload,m_sender, &TcpSendder::onVideoFrame);
    connect(m_ipc, &Ipcreceiver::dht11Data,m_sender, &TcpSendder::onDht11Data);
    connect(m_ipc, &Ipcreceiver::lightData,m_sender, &TcpSendder::onLightData);
    m_ipc->start();
    //启动子线程
    startSensorProcesses();
}

Widget::~Widget()
{
    if(m_camera)
    {
        m_camera->stop(); //run的while退出
        m_camera->wait(); //等线程结束
    }
    if(m_ipc)
    {
        m_ipc->stop();
        m_ipc->wait();
        killSensorProcesses();
    }
    //删除共享文件
    unlink(SHM_FLAG_PATH);
    delete ui;
}

void Widget::onFrameReady(QImage img)
{
    //static int handle_cnt = 0;
    //qDebug() << "handle frame" << handle_cnt++;
    ui->videoLabel->setPixmap(QPixmap::fromImage(img));
    m_camera->releaseFrame();
}

void Widget::onErrorOccurred(QString msg)
{
    QMessageBox::warning(this,"camera err",msg);
}

void Widget::onDht11Data(float temp, float humi)
{
    ui->temp->setText(QString("温度：%1").arg(static_cast<double>(temp), 0,'f',1));
    ui->huimi->setText(QString("湿度 %1").arg(static_cast<double>(humi), 0,'f',1));
}

void Widget::onLightData(float lux)
{
    ui->light->setText(QString("光照：%1").arg(static_cast<double>(lux),0,'f',1));
}

void Widget::startSensorProcesses()
{
    QString appdir = QCoreApplication::applicationDirPath();   //当前程序（text）所在的目录
    QDir::setCurrent(appdir); //当前工作目录切到 appdir
    QStringList procs;        //程序列表   procs = ["dht11_proc", "light_proc"]
    procs << "dht11" << "light";
    for(const QString &name : procs)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            //路径  参数  ./dht11   dht11     constData()转成指针形式，、exec参数类型是指针
            execl(name.toUtf8().constData(),name.toUtf8().constData(),NULL);
            perror("execl");
            _exit(1);
        }
        else if(pid >0)
        {
            m_childPids.append(pid); //记录子进程ID    //列表变成 [1234, 5678, 9999]
            qDebug() << "started" << name << "pid =" << pid;
        }
        else
        {
            perror("fork");
        }
    }
}

void Widget::killSensorProcesses()
{
    for (pid_t pid : m_childPids)
    {
        if (pid > 0)
            kill(pid, SIGTERM);
    }
    m_childPids.clear();
}


void Widget::on_connect_clicked()
{
    if (m_sender->isConnected())
    {
        m_sender->disconnectFromServer();
    }
    else
    {
        m_sender->connectToServer("192.168.1.180", 8888);
        ui->information->setText("连接中...");
        ui->information->setStyleSheet("color: orange;");
    }
}
