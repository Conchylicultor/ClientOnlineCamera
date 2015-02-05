TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_MAKEFILE += build/makefile
QMAKE_CXXFLAGS += -std=c++11

SOURCES += src/main.cpp \
    src/exchangemanager.cpp \
    src/features.cpp \
    src/featuresmanager.cpp \
    src/sequence.cpp

LIBS += -lmosquittopp
LIBS += `pkg-config opencv --libs`

HEADERS += \
    src/exchangemanager.h \
    src/features.h \
    src/featuresmanager.h \
    src/sequence.h
