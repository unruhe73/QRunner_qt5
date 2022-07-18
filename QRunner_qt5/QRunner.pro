TEMPLATE =   app
QT +=   xml widgets

HEADERS =   mainwindow.h \
            projectview.h \
            version.h \
            treewidgetitem.h \
            scriptconf.h \
            scriptqueue.h \
            scriptprocess.h \
            textedit.h \
            lineedit.h \
            scripttree.h \
            filesystemtreeview.h \
            texteditmonitor.h \
            monitorview.h \
            settings.h
SOURCES =   main.cpp \
            projectview.cpp \
            mainwindow.cpp \
            treewidgetitem.cpp \
            scriptconf.cpp \
            scriptqueue.cpp \
            scriptprocess.cpp \
            textedit.cpp \
            lineedit.cpp \
            scripttree.cpp \
            filesystemtreeview.cpp \
            texteditmonitor.cpp \
            monitorview.cpp \
            settings.cpp
TRANSLATIONS =   qrunner_it.ts qrunner_de.ts qrunner_fr.ts
RESOURCES =   qrunner.qrc
