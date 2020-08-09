#-------------------------------------------------
#
# Project created by QtCreator 2020-06-23T20:42:16
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cloudDisk
TEMPLATE = app
RC_ICONS = ./images/logo.ico

SOURCES += main.cpp\
        mainwindow.cpp \
    login.cpp \
    selfwidget/titlewidget.cpp \
    common/common.cpp \
    common/des.c \
    common/logininfoinstance.cpp \
    buttongroup.cpp \
    myfilewg.cpp \
    sharelist.cpp \
    common/uploadtask.cpp \
    selfwidget/dataprogress.cpp \
    common/downloadlayout.cpp \
    common/uploadlayout.cpp \
    selfwidget/filepropertyinfo.cpp \
    common/downloadtask.cpp \
    transfer.cpp \
    rankinglist.cpp \
    selfwidget/mymenu.cpp \
    common/httpstatus.cpp

HEADERS  += mainwindow.h \
    login.h \
    selfwidget/titlewidget.h \
    global.h \
    common/common.h \
    common/des.h \
    common/logininfoinstance.h \
    buttongroup.h \
    myfilewg.h \
    sharelist.h \
    common/uploadtask.h \
    selfwidget/dataprogress.h \
    common/downloadlayout.h \
    common/uploadlayout.h \
    selfwidget/filepropertyinfo.h \
    common/downloadtask.h \
    transfer.h \
    rankinglist.h \
    selfwidget/mymenu.h \
    common/httpstatus.h

FORMS    += mainwindow.ui \
    login.ui \
    selfwidget/titlewidget.ui \
    buttongroup.ui \
    myfilewg.ui \
    sharelist.ui \
    selfwidget/dataprogress.ui \
    selfwidget/filepropertyinfo.ui \
    transfer.ui \
    rankinglist.ui

RESOURCES += \
    resource.qrc
