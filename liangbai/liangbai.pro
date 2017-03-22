#-------------------------------------------------
#
# Project created by QtCreator 2016-07-30T15:34:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = liangbai
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialog.cpp

HEADERS  += mainwindow.h \
    dialog.h

FORMS    += mainwindow.ui \
    dialog.ui

QMAKE_CXXFLAGS += -lsqlite3
QMAKE_CXXFLAGS_RELEASE += -lsqlite3
QMAKE_CXXFLAGS_DEBUG += -lsqlite3
QMAKE_CFLAGS += -lsqlite3
QMAKE_CFLAGS_RELEASE += -lsqlite3
QMAKE_CFLAGS_DEBUG += -lsqlite3
LIBS += -lsqlite3

RESOURCES += \
    png.qrc
