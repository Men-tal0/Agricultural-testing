#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "tcpreseiver.h"

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
    void onVideoFrame(QImage img);
    void onDht11Data(float temp, float humi);
    void onLightData(float lux);
    void onClientConnected(QString ip);
    void onClientDisconnected();
private:
    Ui::Widget *ui;
    tcpreseiver *m_receiver;
};
#endif // WIDGET_H
