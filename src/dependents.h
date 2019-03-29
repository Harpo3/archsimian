#ifndef DEPENDENTS_H
#define DEPENDENTS_H
#include <string>
#include <QString>

//Declarations for functions needed to identify the run state of
// third party apps by name and return a bool with the result

int getProcIdByName(std::string x);
bool isAppRunning(std::string x);
bool isLibRefreshNeeded(const QString &s_mmBackupDBDir);
std::string getMMdbDate(const QString &s_mmBackupDBDir);
std::string getLastTableDate();
#endif // DEPENDENTS_H
