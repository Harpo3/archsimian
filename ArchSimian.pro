#-------------------------------------------------
#
# Project created by QtCreator 2018-11-07T10:47:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = archsimian
TEMPLATE = app
VERSION = 1.05
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

CONFIG += c++14

SOURCES += \
    ../../id3lib-3.8.3/src/c_wrapper.cpp \
    ../../id3lib-3.8.3/src/field.cpp \
    ../../id3lib-3.8.3/src/field_binary.cpp \
    ../../id3lib-3.8.3/src/field_integer.cpp \
    ../../id3lib-3.8.3/src/field_string_ascii.cpp \
    ../../id3lib-3.8.3/src/field_string_unicode.cpp \
    ../../id3lib-3.8.3/src/frame.cpp \
    ../../id3lib-3.8.3/src/frame_impl.cpp \
    ../../id3lib-3.8.3/src/frame_parse.cpp \
    ../../id3lib-3.8.3/src/frame_render.cpp \
    ../../id3lib-3.8.3/src/globals.cpp \
    ../../id3lib-3.8.3/src/header.cpp \
    ../../id3lib-3.8.3/src/header_frame.cpp \
    ../../id3lib-3.8.3/src/header_tag.cpp \
    ../../id3lib-3.8.3/src/helpers.cpp \
    ../../id3lib-3.8.3/src/io.cpp \
    ../../id3lib-3.8.3/src/io_decorators.cpp \
    ../../id3lib-3.8.3/src/io_helpers.cpp \
    ../../id3lib-3.8.3/src/misc_support.cpp \
    ../../id3lib-3.8.3/src/mp3_parse.cpp \
    ../../id3lib-3.8.3/src/readers.cpp \
    ../../id3lib-3.8.3/src/spec.cpp \
    ../../id3lib-3.8.3/src/tag.cpp \
    ../../id3lib-3.8.3/src/tag_file.cpp \
    ../../id3lib-3.8.3/src/tag_find.cpp \
    ../../id3lib-3.8.3/src/tag_impl.cpp \
    ../../id3lib-3.8.3/src/tag_parse.cpp \
    ../../id3lib-3.8.3/src/tag_parse_lyrics3.cpp \
    ../../id3lib-3.8.3/src/tag_parse_musicmatch.cpp \
    ../../id3lib-3.8.3/src/tag_parse_v1.cpp \
    ../../id3lib-3.8.3/src/tag_render.cpp \
    ../../id3lib-3.8.3/src/utils.cpp \
    ../../id3lib-3.8.3/src/writers.cpp \
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
    ../../id3lib-3.8.3/include/id3.h \
    ../../id3lib-3.8.3/src/field_def.h \
    ../../id3lib-3.8.3/src/field_impl.h \
    ../../id3lib-3.8.3/src/flags.h \
    ../../id3lib-3.8.3/src/frame_def.h \
    ../../id3lib-3.8.3/src/frame_impl.h \
    ../../id3lib-3.8.3/src/header.h \
    ../../id3lib-3.8.3/src/header_frame.h \
    ../../id3lib-3.8.3/src/header_tag.h \
    ../../id3lib-3.8.3/src/mp3_header.h \
    ../../id3lib-3.8.3/src/spec.h \
    ../../id3lib-3.8.3/src/tag_impl.h \
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
               /home/lpc123/scripts/cplus/id3lib-3.8.3/include
               /home/lpc123/scripts/cplus/id3lib-3.8.3/src

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

target.path = $$[QT_INSTALL_BINS]
INSTALLS += target

RESOURCES += \
    systray.qrc \    

DISTFILES +=
