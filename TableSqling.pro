QT += core widgets sql printsupport

CONFIG += c++17

TARGET = SQLTableEditor
TEMPLATE = app

# Source files
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sqlworker.cpp

# Header files
HEADERS += \
    mainwindow.h \
    sqlworker.h

# Additional clean files
QMAKE_CLEAN += $(TARGET)

# Compiler flags for professional development
QMAKE_CXXFLAGS += -Wall -Wextra -pedantic
