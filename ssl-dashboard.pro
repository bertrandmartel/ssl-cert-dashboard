#-------------------------------------------------
#
# Project created by QtCreator 2015-09-03T00:13:52
#
#-------------------------------------------------

QT       += core
QT       += network
QT       -= gui

TARGET = ssl-dashboard
CONFIG += console
CONFIG -= app_bundle
CONFIG += qt debug

INCLUDEPATH += $$PWD/libs
QMAKE_LIBDIR     += $$PWD/libs

LIBS        += -lmongoclient -lboost_thread -lboost_system -lboost_filesystem -lboost_program_options
LIBS        += -lhttpServer
LIBS        += -lhttpdecoder
LIBS        += -ldigestauthsession
LIBS        += -lqjson
LIBS        += -lsslgen
LIBS        += -lssl
LIBS        += -lcrypto
LIBS        += -ldl


DESTDIR = release

TEMPLATE = app

OBJECTS_DIR=bin

SOURCES += server/*.cpp \
           utils/*.cpp

HEADERS += server/*.h \
           utils/*.h

QMAKE_CLEAN += -r $${PWD}/$${DESTDIR}

QMAKE_POST_LINK +=$$quote(rsync -avm --include=*/ --include=*.h --exclude=* $${PWD}/$${SOURCES_DIR}/ $${PWD}/$${DESTDIR})
