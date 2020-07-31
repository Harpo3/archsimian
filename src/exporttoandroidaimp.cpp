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

void exportPlaylistToAndroidAIMP(){
    // open cleanedplaylist.txt as read file to convert the path of each track from
    // /mnt/vboxfiles/music/ to /storage/emulated/0/Download/ , then write each to cleanedplaylist.m3u8
    // then, copy resulting playlist to android device

    // First, open readfile
    std::cout << "exportPlaylistToAndroidAIMP: Starting..." << std::endl;
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::ifstream linuxfilepath;
    linuxfilepath.open (appDataPathstr.toStdString()+"/cleanedplaylist.txt");
    if (linuxfilepath.is_open()) {linuxfilepath.close();}
    else {std::cout << "exportPlaylistToAndroidAIMP: Error opening cleanedplaylist.txt file ." << std::endl;}
    std::string playlist = appDataPathstr.toStdString()+"/cleanedplaylist.txt"; // now we can use it as input file
    std::ifstream readlist(playlist);

    // Next, open write file

    std::ofstream ofs1; //open the cleanedplaylist.m3u8 file for writing.
    ofs1.open(appDataPathstr.toStdString()+"/cleanedplaylist.m3u8");

    // Start while loop to find string /mnt/vboxfiles/music/ and replace it with /storage/emulated/0/Download/

    std::string str1;
    std::string findstring("/mnt/vboxfiles/music/*/*/");
    std::string replacestring("/storage/emulated/0/Download/");
     //std::string lastplayedmarker ("playbackcomplete");
     //std::size_t found = str1.find(lastplayedmarker);
    std::cout << "exportPlaylistToAndroidAIMP: Starting while loop..." << std::endl;

    while(getline(readlist, str1)){
        //read data from file object and put it into string.
        //std::cout << str1 << std::endl;   //print the data of the string
        // get tokens and get variables if partial string matches, save variables for subsequent 2 lines for two more variables
        // replace string with android path
        // find position of sixth / delimiter
        int findstringlength = positionOfXthDelimChar(str1, 5);
        std::cout<< "positionOfXthDelimChar is: "<< findstringlength << std::endl;
        str1.replace(0,findstringlength+2,replacestring);
        std::cout<< "Fixed str is: "<< str1 << std::endl;
        ofs1 << str1 << "\n";
    }

    std::cout<< "Finished loop"<< std::endl;

    ofs1.close();
    readlist.close();



//    QFile("cleanedplaylist.txt");
//    QFile("cleanedplaylist.m3u8");
//   QFile::copy(appDataPathstr+"cleanedplaylist.txt", appDataPathstr+"cleanedplaylist.m3u8");
    //std::ifstream readFile(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
}

void copyMP3sToAndroidAIMP(){
    // open cleanedplaylist.txt to read each file path and copy each referenced mp3 from music library to
    // /storage/emulated/0/Download/ on android device
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/";
    QFile("cleanedplaylist.txt");
    QFile("cleanedplaylist.m3u8");
    QFile::copy(appDataPathstr+"cleanedplaylist.txt", appDataPathstr+"cleanedplaylist.m3u8");
    QString androidMP3Pathstr = "/run/user/1000/5292a4c9dda89b88/primary/Android/Download"; // destination for mp3s on android device with AIMP
}
