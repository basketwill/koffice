# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./shell_qt2
# Target is an application:  kugar

SOURCES += kugarmain.cpp \
           main.cpp 
HEADERS += kugarmain.h 
FORMS += kugarmainbase.ui 
TEMPLATE = app 
CONFIG += release \
          warn_on \
          qt \
          thread 
TARGET = kugar 
QMAKE_LIBDIR = ../lib 
MAKEFILE = Makefile.qt 
INCLUDEPATH = ../lib 
LIBS += -lkugar 
