#include <QFile>
#include <QDir>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "constants.h"

namespace userconfig
{
//    extern QString musiclibrarydirname;
//    extern QString mmbackuppldirname;
//    extern QString mmbackupdbdirname;

    std::string getConfigEntry(int z) // z: 1=music lib, 3=playlist, 5=mm.db dir
    {
        // Check for existence of user configuration file (archsimian.conf).
                std::string configEntryName;
                std::ifstream x;
                x.open(Constants::userFileName);
                if (!x){   // check to ensure the file opened
                    std::cerr << "Uh oh, archsimian.conf could not be opened for reading!" << std::endl;
                    exit(1);
                    }
                std::string y;
                // Get the location of the archsimian.conf entry
                for (int lineno = 0; getline (x,y) && lineno < 6; lineno++) {
                    if (lineno == z) {configEntryName = y;}
                }
                return configEntryName;
    }


    void load_config_file()
    // If user config has already been set, read the config file into the variables,
    // then load the existing locations from archsimian.conf
   {        
        if (const int configSetupResult = 1) {
        std::string line;
        std::ifstream myFile (Constants::userFileName);  // Open the config file: archsimian.conf
        for (int lineno = 0; getline (myFile,line) && lineno < 6; lineno++)
        {
            if (lineno == 1) // "1" is the second line of the archsimian.conf file
                // Get the location of music library from archsimian.conf to set QString musiclibrarydirname
                {QString musiclibrarydirname = QString::fromStdString(line);
                std::cout << line << ".\n";
                std::cout << musiclibrarydirname.toStdString() << ".\n";
     }
            else if (lineno == 3)
                {QString mmbackuppldirname = QString::fromStdString(line);
                std::cout << line << ".\n";
                std::cout << mmbackuppldirname.toStdString()<< ".\n";
                    }
            else if (lineno == 5)
                {QString mmbackupdbdirname = QString::fromStdString(line);
                std::cout << line << ".\n";
                std::cout << mmbackupdbdirname.toStdString()<< ".\n";
                }
        }
 //       std::cout << "\nNow let's test whether the variable values persist outside the function.\n";
//        std::cout << "musiclibrarydirname: " << musiclibrarydirname.toStdString() << ".\n";
//        std::cout << "mmbackuppldirname: " << mmbackuppldirname.toStdString() << ".\n";
//        std::cout << "mmbackupdbdirname: " << mmbackupdbdirname.toStdString() << ".\n";
    }
}

//Function to determine whether a file exists
int is_file_exist(std::string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

// Function setConfig to check for existence of user configuration file (archsimian.conf). If it does not exist, create
// bool used to prompt the user to set it up, then create the empty file. If it exists, continue.
int isConfigSetup()
{
    std::ifstream x (Constants::userFileName);
    if (x.is_open())   {
            x.close();
            return 1;
    }
    else {
            std::cerr << "Couldn't open config file. Setting the bool and creating one.\n";
            std::ofstream x;
            x.open(Constants::userFileName);
            x.close();
            return 0;

    }
}
}
