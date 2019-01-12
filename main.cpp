#include "archsimian.h"
#include "dependents.h"
#include "userconfig.h"
#include "constants.h"

int main(int argc,char* argv[])
{
    //*************************
    // USER CONFIGURATION
    //************************
    // Check the condition of user configuration using the isConfigSetup function
    static int s_isConfigSetResult(userconfig::isConfigSetup());// Call the isConfigSetup function to check if config has been set up

    // If user config not yet set up, launch GUI for user setup
    if (s_isConfigSetResult == 0) { // Evaluate and run gui for user config setup, if result is 0, otherwise continue
        std::cout << "The configuration is not set up: " << s_isConfigSetResult << ". Starting configuration setup in gui." << std::endl;
        QApplication mainapp(argc, argv);
        ArchSimian guiWindow;
        guiWindow.show(); // This launches the user interface (UI) for configuration
        mainapp.exec();
    }
    // Section to launch GUI; uncomment last four lines below to enable
    if (s_isConfigSetResult == 1) { //If user config setup was already run (result is 1) run GUI app here
        // which after the sts and file data have been loaded
        // GUI currently not disabled
            QApplication mainapp(argc, argv);
            ArchSimian guiWindow;
         guiWindow.show();   // This launches the user interface (UI)
        mainapp.exec();
    }
    return 0;
}
