TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    client.cpp

HEADERS += \
    client.h \
    rio.h

LIBS += -lpthread
