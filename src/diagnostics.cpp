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
    QString appDataPathstr = QDir::homePath() + "/.config/" + QApplication::applicationName();
    std::string logd = appDataPathstr.toStdString()+"/archsimian.conf";
    removeAppData(logd);
    // Start log file
    Logger(" ************************************************ ");
    Logger(" ********** ArchSimian Diagnostics Log ********** ");
    Logger(" ************************************************ ");
    Logger("Looking for ArchSimian configuration file...");
    // Extract the paths used for mmBackupDBDir (mmdbPath), musicLibraryDir (musiclibPath) & mmPlaylistDir (pldirPath)
    bool configExist = doesFileExist2 (logd);
    if (configExist == false){
        Logger ("***ERROR*** archsimian.conf was not found." "\n\t\t\t\t\t\t\t\t\tComplete 'Settings' tab in "
                                                                                               "Archsimian then run diagnostics again.");
        qApp->quit();
    }
    if (configExist == true){
        Logger ("archsimian.conf found at: " + logd);
    }
    std::ifstream file(logd);
    std::string strconfig;
    std::string mmdbPath;
    std::string musiclibPath;
    std::string pldirPath;
    Logger ("Verifying files and directories as specified in configuration...");
    while (std::getline(file, strconfig)) {
        std::vector<std::string> splittedStrings = split(strconfig, '=');
        if (splittedStrings[0] == "mmBackupDBDir") {
            Logger("Looking for MediaMonkey backup directory and database (MM.DB) file...");
            mmdbPath = splittedStrings[1];
            configExist = doesFileExist2 (mmdbPath + "/MM.DB");
            if (configExist == false){
                Logger ("*****************************************************");
                Logger ("***ERROR*** MM.DB was NOT FOUND at " + mmdbPath + ". \n\t\t\t\t\t\t\t\t\tBackup your MediaMonkey "
                                                                                           "database to this location, then run diagnostics again.");
                Logger ("*****************************************************");
                qApp->quit();
            }
            if (configExist == true){
                Logger ("MM.DB FOUND at " + mmdbPath);
            }
            continue;}
        else if (splittedStrings[0] == "musicLibraryDir") {
            Logger("Looking for music library...");
            musiclibPath = splittedStrings[1];
            Logger("Music library directory FOUND at " + musiclibPath+ ".\n\n\t\t\t\t\t\t\t\t\tChecking the directory structure...\n");
            if (musiclibPath == ""){
                Logger ("*****************************************************");
                Logger ("***ERROR*** Music library directory was NOT FOUND " + mmdbPath + ". \n\t\t\t\t\t\t\t\t\tCheck the location "
                                                                                           "of your music library, then run diagnostics again.");
                Logger ("*****************************************************");
                qApp->quit();
            }
            //std::cout << musiclibPath << "\n";
            std::string  dirdepth = GetStdoutFromCommand("find "+musiclibPath+" -depth -printf '%d\n' | sort -rn | head -1");
            int i = std::stoi(dirdepth);
            if (i == 2){
                Logger("Music library directory depth is CORRECT at " + dirdepth + ".");
            }
            else {
                Logger ("*****************************************************");
                Logger("[REVIEW REQUIRED] Music library directory depth may NOT be correct. ");
                Logger ("*****************************************************");
                Logger("\n\t\t\t\t\t\t\t\t\tThere should be 2 subdirectories, artist and album only. \n\t\t\t\t\t\t\t\t\tInstead the search found: " + dirdepth);
                std::string dirsearchoutput = GetStdoutFromCommand("find "+musiclibPath+" -mindepth 3 -maxdepth 10 -type d");
                Logger("If the above result was 3, AND the ONLY entry below refers to 'System Volume Information', the result is OK."
                       "\n\t\t\t\t\t\t\t\t\tOtherwise, revise the music folders "
                       "shown:\n\n\t\t\t\t\t\t\t\t\t " + dirsearchoutput);
            }
            std::string countm ("\"*.mp3\"");
            std::string countmp3files = GetStdoutFromCommand("find "+musiclibPath+" -type f -name "+ countm + " -printf x | wc -c");

            int mp3cnt = std::stoi(countmp3files);

            if (mp3cnt < 2000){
                Logger ("*****************************************************");
                Logger("***WARNING*** There were less than 2,000 mp3 files found. \n\t\t\t\t\t\t\t\t\tUncertain whether"
                       " program will work correctly.");
                Logger ("*****************************************************");
                Logger("Number of mp3 files found was "+ countmp3files);
            }
            else {
                Logger("Music library CHECKED and an ACCEPTABLE number of mp3 files were found: " + countmp3files);
            }
            Logger("Checking library for mp3 filenames containing spaces...\n");
            std::string countn ("\"* *.mp3*\"");
            std::string countmp3fspaces = GetStdoutFromCommand("find "+musiclibPath+" -name *"+ countn + " -printf x | wc -c");
            int mp3spccnt = std::stoi(countmp3fspaces);
            if (mp3spccnt > 0){
                Logger ("*****************************************************");
                Logger("***WARNING*** There were mp3s found with spaces: " + countmp3fspaces);
                Logger ("*****************************************************");
                std::string listmp3fspaces = GetStdoutFromCommand("find "+musiclibPath+" -name "+ countn);
                Logger("Revise the following: \n\n" + listmp3fspaces);
            }
            else {
                Logger("Music files CHECKED and no mp3 were found to have spaces: " + countmp3fspaces);
            }
            continue;}
    }
    Logger("CONFIGURATION CHECK COMPLETED." );
}
