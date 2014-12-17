TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11

SOURCES += src/main.cpp \
    src/exchangemanager.cpp

LIBS += -lmosquittopp

HEADERS += \
    src/exchangemanager.h
