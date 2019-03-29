#-------------------------------------------------
#
# Project created by QtCreator 2018-11-07T10:47:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = archsimian
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

SOURCES += \
    src/archsimian.cpp \
    src/basiclibfunctions.cpp \
    src/dependents.cpp \
    src/getplaylist.cpp \
    src/lastplayeddays.cpp \
    src/main.cpp \
    src/playlistfunctions.cpp \
    src/runbashapp.cpp \
    src/writesqlfile.cpp \
    src/getartistexcludes.cpp

HEADERS += \
    src/archsimian.h \
    src/basiclibfunctions.h \
    src/constants.h \
    src/dependents.h \
    src/getplaylist.h \
    src/lastplayeddays.h \
    src/playlistfunctions.h \
    src/runbashapp.h \
    src/writesqlfile.h

FORMS += \
    src/archsimian.ui

INCLUDEPATH += /src
               /images

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

target.path = $$[QT_INSTALL_BINS]
INSTALLS += target

RESOURCES += \
    systray.qrc \
