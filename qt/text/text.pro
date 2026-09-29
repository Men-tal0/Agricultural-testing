QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
contains(QMAKE_CC, .*arm.*) {
    message("=== ARM 编译，使用 OpenCV 3.1 ===")
    INCLUDEPATH += /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/include
    LIBS += -L/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/lib \
            -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs
-lopencv_video
} else {
    message("=== x86 编译，使用 OpenCV 3.4.1 ===")
    INCLUDEPATH += /opt/opencv341/include
    LIBS += -L/opt/opencv341/lib \
            -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs
}
SOURCES += \
    camerathread.cpp \
    ipcreceiver.cpp \
    main.cpp \
    tcpsendder.cpp \
    widget.cpp \
    camera.c
HEADERS += \
    camerathread.h \
    ipcreceiver.h \
    tcpsendder.h \
    widget.h \
    camera.h \
    ../../sensor.h \
    ../../shm_flag.h \
    ../../protocol.h
FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
