#-------------------------------------------------
#
# Project created by QtCreator 2016-02-20T15:32:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = menu
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui

target.path = /usr/local/menu
INSTALLS += target

RESOURCES += \
    resources.qrc
