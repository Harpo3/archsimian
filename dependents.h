#ifndef DEPENDENTS_H
#define DEPENDENTS_H
#include <string>
//Declarations for functions needed to identify the state of
// third party apps required for this program to operate

int getProcIdByName(std::string x);

bool isAppRunning(std::string x);

#endif // DEPENDENTS_H
