#include <QFile>
#include <QDir>
#include <iostream>
#include <fstream>
#include <sys/stat.h>


//Function to determine whether a user configuration file exists

bool is_file_exist(std::string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

