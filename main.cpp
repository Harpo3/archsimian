#include <QSettings>
#include <QApplication>
#include <QCloseEvent>
#include <QtWidgets>
#include "archsimian.h"
#include "dependents.h"
#include "userconfig.h"
#include "constants.h"
#include "runbashapp.h"

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
