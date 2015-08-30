#-------------------------------------------------
#
# Project created by QtCreator 2012-08-28T10:27:52
#
#-------------------------------------------------

QT       += core xml

QT       -= gui

TARGET = QtTinyOrm
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += \
    src/SqlWriter.h \
    src/Parser.h \
    src/DataObjects.h \
    src/ClassWriter.h

SOURCES += \
    src/SqlWriter.cpp \
    src/Parser.cpp \
    src/main.cpp \
    src/ClassWriter.cpp

OTHER_FILES += \
    test.xml

