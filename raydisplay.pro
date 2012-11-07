#-------------------------------------------------
#
# Project created by QtCreator 2012-03-21T20:05:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++0x -mtune=native -march=native -fopenmp

TARGET = raydisplay
TEMPLATE = app

LIBS += -lgomp -lopencv_core -lopencv_highgui -lblob -L../raydisplay/cvblobs -lopencv_imgproc

SOURCES += main.cpp\
        raydisplaywindow.cpp \
    RayDisplayScene.cpp \
    Tracker.cpp

HEADERS  += raydisplaywindow.h \
    RayDisplayScene.h \
    Blob.h \
    Tracker.h

FORMS    += raydisplaywindow.ui
