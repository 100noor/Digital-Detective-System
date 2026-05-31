QT       += core gui widgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET   = DigitalDetective
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    logic.cpp \
    dbmanager.cpp

HEADERS += \
    mainwindow.h \
    logic.h \
    dbmanager.h

FORMS += \
    mainwindow.ui

# Disable deprecated Qt API warnings
DEFINES += QT_DEPRECATED_WARNINGS
