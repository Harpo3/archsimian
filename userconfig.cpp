#include <iostream>
#include <fstream>
//#include <sys/stat.h>
#include "constants.h"

namespace userconfig
{

// Function setConfig to check for existence of user configuration file (archsimian.conf). If it does not exist, create
// bool used to prompt the user to set it up, then create an empty archsimian.conf file. If it exists, continue.
int isConfigSetup()
{
    std::ifstream x (Constants::userFileName);
    if (x.is_open()) {
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

// Retrieves a specific entry in the user configuration file (archsimian.conf).
// for assignment to a local variable
std::string getConfigEntry(int z) // z: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
{
    std::string configEntryName;
    std::ifstream x;
    x.open(Constants::userFileName);
    if (!x){   // check to ensure the file opened
        std::cerr << "Uh oh, archsimian.conf could not be opened for reading!" << std::endl;
        exit(1);
    }
    std::string y;
    // Get the location of the archsimian.conf entry
    for (int lineno = 0; getline (x,y) && lineno < 8; lineno++) {
        if (lineno == z) {configEntryName = y;}
    }
    return configEntryName;
}
}
