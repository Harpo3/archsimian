#include <QFile>
#include <QDir>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

//Function loadPathInfo is used to store the local path of the user, then add
//the directory location where the user's configuration file will be stored

QString loadPathInfo()
{
QString localConfigDirPath = QDir::homePath();
localConfigDirPath.append("/.local/share/archsimian/");
return localConfigDirPath;
}

// Convert localConfigPath from a QString to a std::string

std::string convLocConfDirPath()
{
QString str1 = (loadPathInfo());
QByteArray ba = str1.toLatin1();
const char *c_str2 = ba.data();
//std::cout << c_str2;
return c_str2;
}

//Function to determine whether a user configuration file exists

bool is_file_exist(std::string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

