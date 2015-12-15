#-------------------------------------------------
#
# Project created by QtCreator 2015-12-15T22:34:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MCU_CV
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    player.cpp

HEADERS  += mainwindow.h \
    player.h

FORMS    += mainwindow.ui

INCLUDEPATH += -I/usr/local/include
INCLUDEPATH += -I/usr/local/include/opencv

LIBS += -L/usr/local/lib
LIBS += -lopencv_highgui
LIBS += -lopencv_contrib
LIBS += -lopencv_imgproc
LIBS += -lopencv_core
LIBS += -lopencv_features2d
LIBS += -lopencv_nonfree
LIBS += -lopencv_flann
LIBS += -lopencv_calib3d
