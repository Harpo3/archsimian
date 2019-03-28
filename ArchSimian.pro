#-------------------------------------------------
#
# Project created by QtCreator 2018-11-07T10:47:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = archsimian-git
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
    main.cpp \
    archsimian.cpp \
    dependents.cpp \
    lastplayeddays.cpp \
    runbashapp.cpp \
    getplaylist.cpp \
    writesqlfile.cpp \
    basiclibfunctions.cpp \
    playlistfunctions.cpp

HEADERS += \
    archsimian.h \
    dependents.h \
    lastplayeddays.h \
    constants.h \
    runbashapp.h \
    getplaylist.h \
    writesqlfile.h \
    playlistfunctions.h \
    basiclibfunctions.h

FORMS += \
    archsimian.ui

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

target.path = /usr
INSTALLS += target

RESOURCES += \
    systray.qrc \

DISTFILES += \
    configure.ac \
    Makefile.am
