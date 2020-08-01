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
#include "lastplayeddays.h"
#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <id3/tag.h>

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

void testid3tag(){
    ID3_Tag myTag;
    myTag.Link("/home/lpc123/Downloads/03_-_travis_-_the_weight.mp3");
       ID3_Frame *frame;
       if ( (frame = myTag.Find ( ID3FID_BAND )) )
         {
          char band[ 1024 ];
          frame->Field ( ID3FN_TEXT ).Get ( band, 1024 );
          std::cout << "Band: " << band << std::endl;
         }
        if ( (frame = myTag.Find ( ID3FID_TITLE )) )
          {
           char title[ 1024 ];
           frame->Field ( ID3FN_TEXT ).Get ( title, 1024 );
           std::cout << "Title: " << title << std::endl;
          }
        if ( (frame = myTag.Find ( ID3FID_ALBUM )) )
          {
           char album[ 1024 ];
           frame->Field ( ID3FN_TEXT ).Get ( album, 1024 );
           std::cout << "Album: " << album << std::endl;
          }
        else
         std::cout<<"not found\n";

}

void getLastPlayedDates(){
    //if (Constants::verbose == true) std::cout << "Starting selectTrack function. Rating for next track is " << s_ratingNextTrack << std::endl;

    // Add code here to check kdeconnect setting

    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/";
    std::fstream debuglog;
    debuglog.open ("/run/user/1000/5292a4c9dda89b88/primary/Android/data/com.aimp.player/cache/debug.log");
    //debuglog.open ("debug.log");
    if (debuglog.is_open()) {debuglog.close();}
    else {std::cout << "getLastPlayedDates: Error opening debuglog.txt file." << std::endl;}
    std::string debuglogfile = "/run/user/1000/5292a4c9dda89b88/primary/Android/data/com.aimp.player/cache/debug.log";
    std::ifstream debug(debuglogfile);
    if (!debug.is_open()) {
        std::cout << "getLastPlayedDates: Error opening debug.log." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream dateslist(appDataPathstr.toStdString()+"lastplayeddates.txt");
    std::string str1;
    std::string lastplayedmarker("playbackcomplete");
    std::string begpos("D main\t");
    std::string endpos1("AM");
    std::string endpos2("PM");
    int linecount{0};
    std::string collecteddate{""};
    std::string lastplayedentry{""};
    std::string artistentry{""};
    std::string songentry{""};
    int artistline {0};
    int songline ={0};
    std::string str2{""};
    std::string str3{""};

    while (std::getline(debug, str1)) {
        std::size_t found = str1.find(lastplayedmarker);
        // Beginning of first section
        if (found!=std::string::npos) {  // found!= (not found) means the function did not return string::npos,
            // meaning a match was FOUND for the current line; Next get the date from that same line
            //define start and end positions for date string, then save date string
            str2 = str1;
            std::cout << "Match found at line: " << linecount << '\n';
            std::size_t foundstart = str2.find(begpos);
            std::size_t foundend1 = str2.find(endpos1);
            std::size_t foundend2 = str2.find(endpos2);
            std::size_t foundend = 0;
            if (foundstart!=std::string::npos)
                //std::cout << "first 'position of date' found at: " << foundstart << '\n';
                if (foundend1!=std::string::npos){
                    //std::cout << "end1 'position of date' found at: " << foundend1 << '\n';
                    foundend = foundend1;
                }
            if (foundend2!=std::string::npos){
                //std::cout << "end2 'position of date' found at: " << foundend2 << '\n';
                foundend = foundend2;
            }
            //std::cout << "foundend: " << foundend << '\n';
            //std::cout << "str3: " << str3 << '\n';
            if (str3 == ""){
                str3 = str2.substr (7,foundend-5);
                //std::cout << "Extracted date (str3): " << str3 << '\n';
                collecteddate = str3; // Store the string for date found
            }
            if (str3 != ""){
                artistline = linecount + 1; // Set the line number for artist variable after lastplayed date found
                songline = linecount + 4;  // Set the line number for song title after lastplayed date found
                //std::cout << "artist line #: " << artistline << '\n';
                //std::cout << "song line #: " << songline << '\n';
            }
            continue;
         }
        // Beginning of 2nd section;
        if (found==std::string::npos) {  // found== means the function did return string::npos,
            // meaning a match was NOT FOUND for the current line; Next
            if ((linecount == artistline) && (collecteddate != "")){
                // get artist string if it is two lines after lastplayed date found
                std::size_t strlength = str1.length();
                artistentry = str1.substr (7,strlength-7);
               // std::cout << "extracted artist string is: " << artistentry << '\n';
            }
            if ((linecount == songline) && (collecteddate != "")){
                // get song string if it is two lines after lastplayed date found
                std::size_t strlength = str1.length();
                songentry = str1.substr (6,strlength-6);
                //std::cout << "extracted artist string is: " << songentry << '\n';
            }
            if ((linecount == songline +1) && (collecteddate != "")){
                // If the line after completing assignment of the 3 variables: date, artist and song, save all three into one csv line
                // to export or push into vector, then reset variables and continue going through the log
                lastplayedentry = artistentry + "," + songentry+ "," +collecteddate;
                std::cout << "lastplayedentry string is: " << lastplayedentry << '\n';
                dateslist << lastplayedentry<<"\n";
                collecteddate = "";
                artistline = 0;
                songline = 0;
                str3 = "";
            }
            ++linecount;
        }
    }
    dateslist.close();
    debug.close();
}

void logdateconversion(){
    std::string chkthis = "Jul 26, 2020 1:30:42 PM";
    const char *timestr = chkthis.c_str();

    struct tm result;
    time_t t_of_day;
    strptime(timestr, "%b %d, %Y %r", &result);

    char buf[128];
    strftime(buf, sizeof(buf), "%b %d, %Y %r", &result);

    std::cout << timestr << " -> " << buf << std::endl;

    printf("tm_hour: %d\n",result.tm_hour);
    printf("tm_min:  %d\n",result.tm_min);
    printf("tm_sec:  %d\n",result.tm_sec);
    printf("tm_mon:  %d\n",result.tm_mon);
    printf("tm_year: %d\n",result.tm_year);
    result.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
    t_of_day = mktime(&result);
    printf("seconds since the Epoch: %ld\n", long(t_of_day));
    //double tempconv = double(t_of_day);
    //int newresult = int(getLastPlayedDays(tempconv));
    //std::cout << "newresult = " << newresult << " days since last played." << std::endl;

}
