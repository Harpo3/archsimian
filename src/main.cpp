#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QStandardPaths>
#include "archsimian.h"
#include<iostream>

int main(int argc,char* argv[])
{
        QApplication mainapp(argc, argv);
        // Set application name and directory for application data (/local/share)
        QApplication::setOrganizationName("archsimian");
        QApplication::setApplicationName("archsimian");
        QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
        QDir appDataPath = appDataPathstr;
        if( !appDataPath.exists() )
        {
            appDataPath.mkpath(appDataPathstr);
        }
        QString mydirpath = appDataPath.path();
        std::string convertStdPath = mydirpath.toStdString();
        // Launch GUI (archsimian.cpp, which is the mainwindow file)
        ArchSimian guiWindow;        
        guiWindow.show();        
        QApplication::exec();
    return 0;
}
