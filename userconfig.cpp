#include <QFile>
#include <QDir>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "constants.h"

//Function to determine whether a user configuration file exists
int is_file_exist(std::string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

// Function setConfig to check fo existence of user configuration file (archsimian.conf). If it does not exist, create
// bool used to prompt the user to set it up, then create the empty file. If it exists, continue.
int isConfigSetup()
{
    std::ifstream x (Constants::userFileName);
    if (x.is_open())
    {
            std::cout << "The config file exists!" << '\n';
            x.close();
            return true;
    }
    else {
            std::cerr << "Couldn't open config file. Setting the bool and creating a new one.\n";
            std::ofstream x;
            x.open(Constants::userFileName);
            //x << "Now it has been created and here is some text.";
            x.close();
            return false;
    }
}
