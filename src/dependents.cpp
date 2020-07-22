// This cpp is used to provide functions used to determine the update status of MM.DB.
#include <fstream>
#include <iostream>
#include <QString>
#include <QStandardPaths>
#include <QDir>
#include "constants.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

inline bool doesFileExist (const std::string& name) {
    struct stat buffer{};
    return (stat (name.c_str(), &buffer) == 0);
}

bool isLibRefreshNeeded(const QString &s_mmBackupDBDir)
{
    const std::string existlibname = "ratedlib.dsv";
    bool existResult;
    bool refreshNeededResult{false};
    existResult = doesFileExist(existlibname);// See inline function at top
    std::cout << "File exist result is " << existResult << std::endl;
    refreshNeededResult = true;
    // If the lib file exists, Get the epoch date for the MM.DB file
    // and see which file is older
    if (existResult == 1){
        std::string mmdbdir = s_mmBackupDBDir.toStdString(); // z: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath);
        std::string mmpath = mmdbdir + "/MM.DB";
        struct stat stbuf1{};
        stat(mmpath.c_str(), &stbuf1);
        localtime(&stbuf1.st_mtime); // or gmtime() depending on what you want
        // Now get the date for the ratedlib.csv file
        struct stat stbuf2{};
        stat(existlibname.c_str(), &stbuf2);
        localtime(&stbuf2.st_mtime);
        double dateResult = stbuf1.st_mtime - stbuf2.st_mtime;
        if (dateResult > 0) {
            refreshNeededResult = true;
            if (Constants::kVerbose) std::cout << "MM.DB was recently backed up. Updating library and stats..." << std::endl;
        }
        // If the result is negative, then MM4 has not been updated since the program library was last refreshed. No update is necessary.
        // If positive, need to refresh all library data.
    }
    return refreshNeededResult;
}

std::string getMMdbDate(const QString &s_mmBackupDBDir)
{
    std::string mmdbdir = s_mmBackupDBDir.toStdString(); // z: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath);
    std::string mmpath = mmdbdir + "/MM.DB";
    struct stat stbuf1{};
    struct tm *foo;
    stat(mmpath.c_str(), &stbuf1);
    foo = gmtime(&(stbuf1.st_mtime));
    localtime(&stbuf1.st_mtime);
    std::string a = (std::to_string(foo->tm_mon + 1)) + "/" + (std::to_string(foo->tm_mday)) + "/" + (std::to_string(foo->tm_year + 1900));
    return a;
}

std::string getLastTableDate()
{
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    const std::string existlibname = appDataPathstr.toStdString()+"/cleanlib.dsv";
    struct stat stbuf2{};
    struct tm *foo;
    stat(existlibname.c_str(), &stbuf2);
    foo = gmtime(&(stbuf2.st_mtime));
    localtime(&stbuf2.st_mtime);
    std::string a = (std::to_string(foo->tm_mon + 1)) + "/" + (std::to_string(foo->tm_mday)) + "/" + (std::to_string(foo->tm_year + 1900));
    return a;
}
