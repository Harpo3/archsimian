#ifndef USERCONFIG_H
#define USERCONFIG_H
#include <QString>

//Function to determine whether a user configuration file exists

bool is_file_exist(std::string fileName);

// Function setConfig to check fo existence of user configuration file (archsimian.conf). If it does not exist, create
// bool used to prompt the user to set it up, then create the empty file. If it exists, continue.

int isConfigSetup();

#endif // USERCONFIG_H
