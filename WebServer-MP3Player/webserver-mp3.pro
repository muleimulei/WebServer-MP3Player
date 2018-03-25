TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    server.cpp \
    clientthread.cpp

HEADERS += \
    server.h \
    clientthread.h \
    rio.h

LIBS += -lpthread
