#ifndef DEPENDENTS_H
#define DEPENDENTS_H
#include <string>
//Declarations for functions needed to identify the run state of
// third party apps by name and return a bool with the result

int getProcIdByName(std::string x);

bool isAppRunning(std::string x);

#endif // DEPENDENTS_H
