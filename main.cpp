#include <QSettings>
#include <QApplication>
#include "archsimian.h"

int main(int argc,char* argv[])
{
        QApplication mainapp(argc, argv);        
        mainapp.setOrganizationName("archsimian");
        mainapp.setApplicationName("archsimian");
        ArchSimian guiWindow;
        guiWindow.show();   // This launches the user interface (UI)
        mainapp.exec();
    return 0;
}
