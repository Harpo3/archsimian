#include <QSettings>
#include <QtWidgets>
#include <QPalette>
#include <QStandardPaths>
#include <QApplication>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>
#include <QString>
#include "utilities.h"
#include <QString>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

bool doesFileExist2 (const std::string& name) {
    struct stat buffer{};
    return (stat (name.c_str(), &buffer) == 0);
}
void generateDiagsLog (){
    QString appDataPathstr = QDir::homePath() + "/.local/share/archsimian";
    std::string logd = appDataPathstr.toStdString()+"/diagnosticslog.txt";
    removeAppData(logd);
    // Start log file
    Logger(" ************************************************ ");
    Logger(" ********** ArchSimian Diagnostics Log ********** ");
    Logger(" ************************************************ ");
    // Open configuration file
    QString configFilePath = QStandardPaths::locate(QStandardPaths::AppConfigLocation, QString(), QStandardPaths::LocateDirectory);
    std::string configFile = configFilePath.toStdString()+"archsimian/archsimian.conf";
    std::cout << configFile << std::endl;
    Logger("[Configuration] Looking for ArchSimian configuration file...");
    // Extract the paths used for mmBackupDBDir (mmdbPath), musicLibraryDir (musiclibPath) & mmPlaylistDir (pldirPath)
    bool configExist = doesFileExist2 (configFile);
    if (configExist == false){
        Logger ("[Configuration] ***ERROR*** archsimian.conf was not found at " + configFile + ". \n\t\t\t\t\t\t\t\t\tComplete 'Settings' tab in "
                                                                                               "Archsimian then run diagnostics again.");
        qApp->quit();
    }
    if (configExist == true){
        Logger ("[Configuration] archsimian.conf found at " + configFile);
    }


}
