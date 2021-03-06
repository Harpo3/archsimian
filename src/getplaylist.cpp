#include <sstream>
#include <iostream>
#include <fstream>
#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QFile>
#include "constants.h"
#include "utilities.h"

int musicLibraryDirLen(QString &s_musiclibrarydirname){
    static std::string musicLibraryDir = s_musiclibrarydirname.toStdString(); // in main prgm, need to fix with QSettings variable
    unsigned long cnt = 0;
    for(size_t i=0; musicLibraryDir[i]; i++)
        cnt++;
    return int(cnt);
}
//  Purpose is to remove the m3u headers lines, leaving just the file paths
void getPlaylist(QString &s_defaultPlaylist, const QString &s_musiclibrarydirname, QString &s_musiclibshortened, bool s_topLevelFolderExists){
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::ofstream ofs; //open the cleanedplaylist file for writing with the truncate option to delete the content.
    ofs.open(appDataPathstr.toStdString()+"/cleanedplaylist.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    std::ofstream ofs1; //open the ratedabbr2 file for writing with the truncate option to delete the content.
    ofs1.open(appDataPathstr.toStdString()+"/ratedabbr2.txt", std::ofstream::out | std::ofstream::trunc);
    ofs1.close();
    static std::string s_selectedplaylist = s_defaultPlaylist.toStdString();
    static std::string musiclibdirname = s_musiclibrarydirname.toStdString();
    static std::string musiclibdirshort = s_musiclibshortened.toStdString();
    //std::string playlistFile = s_selectedplaylist;
    std::ifstream readFile(s_defaultPlaylist.toStdString());
    std::ofstream outf(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
    if (!readFile.is_open()) {
        std::cout << "getPlaylist: The readFile did not open. Did you select a backup rather than an exported MediaMonkey "
                     "playlist, or otherwise did you manually set the playlist in archsimian.cof without the full and proper path?";
        Logger ("getPlaylist: The readFile did not open. Did you select a backup rather than an exported MediaMonkey playlist, "
                "or otherwise did you manually set the playlist in archsimian.cof without the full and proper path?");
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    while (std::getline(readFile, line)){
        std::istringstream iss(line); //start with first line
        // Fix the dir string: remove colon, insert library user path, and correct the dir symbol from \ to /
        std::string str = line;
        str.replace(str.begin(),str.begin()+2,"");
        line = str;
        std::string str2 ("\\");
        std::size_t found = line.find(str2);
        if (found!=std::string::npos) {// colon is one char before the first dir symbol
            line.replace(line.find(str2),str2.length(), musiclibdirshort + "/");
            found=line.find("second dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"/");
            found=line.find("third dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"/");
            if (s_topLevelFolderExists == true){
                found=line.find("fourth dir symbol",found+1,1);
                line.replace(line.find(str2),str2.length(),"/");
            }
        }
        outf << line << '\n'; // DO NOT ADD endl here
    }
    readFile.close();
    outf.close();
}

void exportPlaylistToWindows(int &s_musicdirlength, QString &s_mmPlaylistDir, QString &s_winDriveLtr, QString &s_musiclibrarydirname, std::string &playlistpath){
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();        
    if (Constants::kVerbose) std::cout << "exportPlaylistToWindows: playlistpath from function variable = "<<playlistpath<< std::endl;
    static std::string playlistdirname = s_mmPlaylistDir.toStdString();
    if (Constants::kVerbose) std::cout << "exportPlaylistToWindows: playlistdirname = "<<playlistdirname<< std::endl;
    static std::string musicLibraryDir=s_musiclibrarydirname.toStdString();
    if (Constants::kVerbose) std::cout << "exportPlaylistToWindows: musicLibraryDir = "<<musicLibraryDir<< std::endl;
    std::string internalplaylistpath = playlistpath;
    if (Constants::kVerbose) std::cout << "exportPlaylistToWindows: playlistpath from internalplaylistpath variable = "<<internalplaylistpath<< std::endl;
    std::string winDriveLtr = s_winDriveLtr.toStdString();
    std::ifstream readFile(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
    std::ofstream outf(internalplaylistpath);
    if (!readFile.is_open()) {
        std::cout << "exportPlaylistToWindows: The readFile did not open. Did you delete the active playlist?";
        Logger ("exportPlaylistToWindows: The readFile did not open. Did you delete the active playlist?");
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    while (std::getline(readFile, line)){
        std::istringstream iss(line);
        std::string str = line;
        str.replace(str.begin(),str.begin()+s_musicdirlength,winDriveLtr+":\\music");
        line = str;
        std::string str1 (musicLibraryDir);
        std::size_t found1 = line.find(str1);
        if (found1!=std::string::npos){
            str.replace(str.begin(),str.begin(),"");
        }
        std::string str2 ("/");
        std::size_t found = line.find(str2);
        if (found!=std::string::npos) {
            line.replace(line.find(str2),str2.length(),"\\");
            found=line.find("second dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"\\");
            found=line.find("third dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"\\");
        }
        outf << line << '\n'; // DO NOT ADD endl here
    }
    readFile.close();
    outf.close();
}

void exportPlaylistToLinux(){
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/";
    QString appDataPathstr2 = QDir::homePath() + "/";
    QFile("cleanedplaylist.txt");
    QFile("cleanedplaylist.m3u");
    QFile::copy(appDataPathstr+"cleanedplaylist.txt", appDataPathstr2+"cleanedplaylist.m3u");
    //std::ifstream readFile(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
}
