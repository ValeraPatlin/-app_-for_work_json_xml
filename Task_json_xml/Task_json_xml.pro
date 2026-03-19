QT += core gui
QT += sql
QT += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

TARGET = Task_json_xml
TEMPLATE = app


DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        window.cpp \
    table_model.cpp \
    load_data.cpp \
    db_facade.cpp \
    db_worker.cpp

HEADERS += \
        window.h \
    table_model.h \
    load_data.h \
    db_facade.h \
    db_worker.h

FORMS += \
        window.ui
