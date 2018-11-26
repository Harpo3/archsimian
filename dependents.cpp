// This cpp is used to provide functions used to identify the state of any
// third party app (Whether it is running).

#include <dirent.h>
#include <fstream>
#include <iostream>

//Obtain the pid number based on the pgrep name of the app (procName)

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
// 'string x', is or is not running. Use the PID descriptor for the application
// string.

bool isAppRunning(std::string x)
{
//std::string{x};
int pid;
pid = getProcIdByName(x);
if (pid == -1)
{return false;}
else {return true;}
}
