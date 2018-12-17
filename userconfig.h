#ifndef USERCONFIG_H
#define USERCONFIG_H
#include <QString>
#include <iostream>
#include "constants.h"
#include <fstream>
//include <string>
//#include <sstream>

namespace userconfig
{

// Function to check for existence of user configuration file (archsimian.conf). If it does not exist, create
// bool used to prompt the user to set it up, then create an empty archsimian.conf file. If it exists, continue.
int isConfigSetup();

// Retrieves a specific entry in the user configuration file (archsimian.conf).
// for assignment to a local variable
std::string getConfigEntry(int z); // z: 1=music lib, 3=playlist, 5=mm.db dir

}
#endif // USERCONFIG_H
