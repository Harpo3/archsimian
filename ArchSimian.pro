#-------------------------------------------------
#
# Project created by QtCreator 2018-11-07T10:47:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = archsimian
TEMPLATE = app
VERSION = 1.10
DEFINES += APP_VERSION=$$VERSION

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
    src/albumidandselect.cpp \
    src/archsimian.cpp \
    src/basiclibfunctions.cpp \
    src/buildalbumexcllibrary.cpp \
    src/code1.cpp \
    src/dependents.cpp \
    src/diagnostics.cpp \
    src/exporttoandroidaimp.cpp \
    src/getartistadjustedcount.cpp \
    src/getplaylist.cpp \
    src/lastplayeddays.cpp \
    src/main.cpp \
    src/playlistcontentdialog.cpp \
    src/playlistfunctions.cpp \
    src/utilities.cpp \
    src/writesqlfile.cpp \
    src/getartistexcludes.cpp

HEADERS += \
    src/albumidandselect.h \
    src/archsimian.h \
    src/basiclibfunctions.h \
    src/buildalbumexcllibrary.h \
    src/code1.h \
    src/constants.h \
    src/dependents.h \
    src/diagnostics.h \
    src/exporttoandroidaimp.h \
    src/getartistadjustedcount.h \
    src/getartistexcludes.h \
    src/getplaylist.h \
    src/lastplayeddays.h \
    src/playlistcontentdialog.h \
    src/playlistfunctions.h \
    src/utilities.h \
    src/writesqlfile.h

FORMS += \
    src/archsimian.ui \
    src/playlistcontentdialog.ui

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

DISTFILES += \


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../lib/release/ -lid3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../lib/debug/ -lid3
else:unix: LIBS += -L$$PWD/../../../../../../lib/ -lid3

INCLUDEPATH += $$PWD/../../../../../../usr/lib
DEPENDPATH += $$PWD/../../../../../../usr/lib
