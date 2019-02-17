// This cpp is used to provide functions used to identify the state of any
// third party app (Whether it is running) and update status of MM.DB.

#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "userconfig.h"
#include <QString>

inline bool doesFileExist (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

//Obtain the pid number based on the pgrep name of the app (std::string x)
int getProcIdByName(std::string x)
{
    int pid = -1;

    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != nullptr)
    {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp)))
        {
            // Skip non-numeric entries
            int id = atoi(dirp->d_name);
            if (id > 0)
            {
                // Read contents of virtual /proc/{pid}/cmdline file
                std::string cmdPath = std::string("/proc/") + dirp->d_name + "/cmdline";
                std::ifstream cmdFile(cmdPath.c_str());
                std::string cmdLine;
                getline(cmdFile, cmdLine);
                if (!cmdLine.empty())
                {
                    // Keep first cmdline item which contains the program path
                    size_t pos = cmdLine.find('\0');
                    if (pos != std::string::npos)
                        cmdLine = cmdLine.substr(0, pos);
                    // Keep program name only, removing the path
                    pos = cmdLine.rfind('/');
                    if (pos != std::string::npos)
                        cmdLine = cmdLine.substr(pos + 1);
                    // Compare against requested process name
                    if (x == cmdLine)
                        pid = id;
                }
            }
        }
    }

    closedir(dp);
    return pid;
}

// Return a bool indicating whether an application, expressed in the command as
// std::string x, is or is not running. Use the PID descriptor for the application
// string.
bool isAppRunning(std::string x)
{
    int pid;
    pid = getProcIdByName(x);
    if (pid == -1)
    {return false;}
    else {return true;}
}

bool isLibRefreshNeeded(const QString &s_mmBackupDBDir)
{
    const std::string existlibname = "ratedlib.dsv";
    bool existResult;
    bool refreshNeededResult{0};
    existResult = doesFileExist(existlibname);// See inline function at top
    std::cout << "File exist result is " << existResult << std::endl;
    refreshNeededResult = 1;
    // If the lib file exists, Get the epoch date for the MM.DB file
    // and see which file is older
    if (existResult == 1){
        std::string mmdbdir = s_mmBackupDBDir.toStdString(); // z: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath);
        std::string mmpath = mmdbdir + "/MM.DB";
        struct stat stbuf1;
        stat(mmpath.c_str(), &stbuf1);
        localtime(&stbuf1.st_mtime); // or gmtime() depending on what you want
        //printf("Modification time for MM.DB is %ld\n",stbuf1.st_mtime);
        //std::cout << "MM.DB is " << stbuf1.st_mtime << std::endl;
        // Now get the date for the ratedlib.csv file
        struct stat stbuf2;
        stat(existlibname.c_str(), &stbuf2);
        localtime(&stbuf2.st_mtime);
        //printf("Modification time for ratedlib.csv is %ld\n",stbuf2.st_mtime);
        //std::cout << "ratedlib.csv is " << stbuf2.st_mtime << std::endl;
        double dateResult = stbuf1.st_mtime - stbuf2.st_mtime;
        if (dateResult > 0) {
            refreshNeededResult = 1;
            if (Constants::verbose == true) std::cout << "MM.DB was recently backed up. Updating library and stats..." << std::endl;
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
    struct stat stbuf1;
    struct tm *foo;
    stat(mmpath.c_str(), &stbuf1);
    foo = gmtime(&(stbuf1.st_mtime));
    localtime(&stbuf1.st_mtime);
    if (Constants::verbose == true) {
        printf("Year: %d\n", foo->tm_year + 1900);
        printf("Month: %d\n", foo->tm_mon + 1);
        printf("Day: %d\n", foo->tm_mday);
    }
    std::string a = (std::to_string(foo->tm_mon + 1)) + "/" + (std::to_string(foo->tm_mday)) + "/" + (std::to_string(foo->tm_year + 1900));
    return a;
}

std::string getLastTableDate()
{
    const std::string existlibname = "cleanlib.dsv";
    struct stat stbuf2;
    struct tm *foo;
    stat(existlibname.c_str(), &stbuf2);
    foo = gmtime(&(stbuf2.st_mtime));
    localtime(&stbuf2.st_mtime);
    if (Constants::verbose == true) {
    printf("Year: %d\n", foo->tm_year + 1900);
    printf("Month: %d\n", foo->tm_mon + 1);
    printf("Day: %d\n", foo->tm_mday);
    }
    std::string a = (std::to_string(foo->tm_mon + 1)) + "/" + (std::to_string(foo->tm_mday)) + "/" + (std::to_string(foo->tm_year + 1900));
    return a;
}
