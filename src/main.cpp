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
        QApplication::setOrganizationName("archsimian");
        QApplication::setApplicationName("archsimian");

        QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
        QDir appDataPath = appDataPathstr;
        if( ! appDataPath.exists() )
        {
            appDataPath.mkpath(appDataPathstr);
        }
        QString mydirpath = appDataPath.path();
        std::string convertStdPath = mydirpath.toStdString();
        //std::cout <<convertStdPath<< std::endl;
        ArchSimian guiWindow;
        guiWindow.show();   // This launches the user interface (UI)
        QApplication::exec();
    return 0;
}
