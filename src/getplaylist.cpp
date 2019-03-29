#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <iostream>
#include "constants.h"

int musicLibraryDirLen(QString &s_musiclibrarydirname)
{
static std::string musicLibraryDir = s_musiclibrarydirname.toStdString(); // in main prgm, need to fix with QSettings variable
// std::string str = "anyString";
 unsigned long cnt = 0;
 for(size_t i=0; musicLibraryDir[i]; i++)
    cnt++;
 return int(cnt);
}

std::istream& safeGetline(std::istream& is, std::string& t){
    t.clear();
    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.
    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();
    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case std::streambuf::traits_type::eof():
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:

            t += static_cast<char>(c);
        }
    }
}

int cstyleStringCount(std::string path){
    std::ifstream ifs(path.c_str());
    if(!ifs) {
        std::cout << "Failed to open the file." << std::endl;
        return EXIT_FAILURE;
    }
    int n = 0;
    std::string t;
    while(!safeGetline(ifs, t).eof())
        ++n;
    //std::cout << "The file contains " << n << " lines." << std::endl;
    return n;
}

void getPlaylist(const QString &s_defaultPlaylist, const QString &s_musiclibrarydirname){   //  Purpose is to remove the m3u headers lines, leaving just the file path
    // need to change config management for selected playlist and music library directory to QSettings format
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    static std::string s_selectedplaylist = s_defaultPlaylist.toStdString();
    static std::string musiclibdirname = s_musiclibrarydirname.toStdString();
    std::string playlistFile = s_selectedplaylist;
    std::ifstream readFile(playlistFile);
    std::ofstream outf(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
    if (!readFile.is_open()) {
        std::cout << "getPlaylist: The readFile did not open. Did you manually export the MediaMonkey playlist?";
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    //std::getline(readFile, line); //skip first line
    while (std::getline(readFile, line)){
        std::istringstream iss(line); //start with first line
        // Fix the dir string: remove colon, insert library user path, and correct the dir symbol from \ to /
        std::string str = line;
        str.replace(str.begin(),str.begin()+2,"");
        line = str;
        std::string str2 ("\\");
        std::size_t found = line.find(str2);
        if (found!=std::string::npos) {// colon is one char before the first dir symbol
            line.replace(line.find(str2),str2.length(),musiclibdirname + "/");
            found=line.find("second dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"/");
            found=line.find("third dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"/");
        }
        outf << line << '\n'; // DO NOT ADD endl here
    }
    readFile.close();
    outf.close();
}

void getWindowsDriveLtr(QString &s_defaultPlaylist, QString *s_winDriveLtr)
{    
    static std::string playlist = s_defaultPlaylist.toStdString();
    std::ifstream readFile(playlist);
    std::string line;
    std::getline(readFile, line); //skip first line
    std::getline(readFile, line);
    std::istringstream iss(line); //start with first line
    *s_winDriveLtr = line.front();
    readFile.close();
}

void exportPlaylistToWindows(int &s_musicdirlength, QString &s_mmPlaylistDir, QString &s_defaultPlaylist, QString &s_winDriveLtr, QString &s_musiclibrarydirname){
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    static std::string playlistpath = s_defaultPlaylist.toStdString();
    static std::string playlistdirname = s_mmPlaylistDir.toStdString();
    static std::string musicLibraryDir=s_musiclibrarydirname.toStdString();
    std::string winDriveLtr = s_winDriveLtr.toStdString(); // in main prgm, need to fix with QSettings variable
    //std::string cleanedPlaylist{Constants::cleanedPlaylist}; // in main prgm, instead reference Constants::cleanedPlaylist
    std::ifstream readFile(appDataPathstr.toStdString()+"/cleanedplaylist.txt"); //fixed here on 03/24/2019
    std::ofstream outf(playlistpath);
    if (!readFile.is_open()) {
        std::cout << "exportPlaylistToWindows: The readFile did not open. Did you delete the active playlist?";
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    while (std::getline(readFile, line)){
        std::istringstream iss(line);
        std::string str = line;
        str.replace(str.begin(),str.begin()+s_musicdirlength,winDriveLtr+":");
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