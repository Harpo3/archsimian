#include <QSettings>
#include <QApplication>
#include <QCloseEvent>
#include <QtWidgets>
#include "archsimian.h"
#include "dependents.h"
#include "userconfig.h"
#include "constants.h"
#include "runbashapp.h"
#include "dialogsetup.h"

int main(int argc,char* argv[])
{
    //*************************
    // USER CONFIGURATION
    //************************
    // Check the condition of user configuration using the isConfigSetup function
    //static int s_isConfigSetResult(userconfig::isConfigSetup());// Call the isConfigSetup function to check if config has been set up

    // If user config not yet set up, launch GUI for user setup
    //if (s_isConfigSetResult == 0) { // Evaluate and run gui for user config setup, if result is 0, otherwise continue
    //    std::cout << "The configuration is not set up: " << s_isConfigSetResult << ". Starting configuration setup in gui." << std::endl;
    //    QApplication mainapp(argc, argv);
    //    mainapp.setOrganizationName("archsimian");
    //    mainapp.setApplicationName("archsimian");

   //     ArchSimian guiWindow;
   //     guiWindow.show(); // This launches the user interface (UI) for configuration
   //     mainapp.exec();
   // }
    // Section to launch GUI with data
   // if (s_isConfigSetResult == 1) { //If user config setup was already run (s_isConfigSetResult is 1) run GUI app
        QApplication mainapp(argc, argv);        
        mainapp.setOrganizationName("archsimian");
        mainapp.setApplicationName("archsimian");
//        QWidget setupwindow;
//        DialogSetup loadSettings1;
        ArchSimian guiWindow;
        guiWindow.show();   // This launches the user interface (UI)
//        DialogSetup usersetup;
//        usersetup.setModal(true);
//        usersetup.show();
        mainapp.exec();

    return 0;
}
