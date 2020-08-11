#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
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
#include <string.h>
#include <id3/tag.h>
//#include <id3/misc_support.h>
//#include "id3/id3lib_streams.h"

// Convert standard (format: "Jul 26, 2020 1:30:42 PM") date/time string variable and return SQL time
double logdateconversion(std::string chkthis){
    const char *timestr = chkthis.c_str();
    struct tm result;
    time_t t_of_day;
    strptime(timestr, "%b %d, %Y %r", &result);
    char buf[128];
    strftime(buf, sizeof(buf), "%b %d, %Y %r", &result);
    result.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
    t_of_day = mktime(&result);
    double sqltime = (double(t_of_day)/86400)+25569;
    return sqltime;
}

void removeMP3sSyncthing(){
    // Remove all existing mp3s from syncthing directory
    QString syncthingPathstr = QDir::homePath() + "/Sync/"; // Set syncthing path
    QDir dir(syncthingPathstr, {"*.mp3"});
    for(const QString & filename: dir.entryList()){
        dir.remove(filename);
    }
}

void syncPlaylistWithSyncthing(){
    // Open cleanedplaylist.txt as read file to convert the path of each track from
    // "/mnt/vboxfiles/music/" to "/storage/emulated/0/Download/" - then write each to cleanedplaylist.m3u8
    // and then, copy the resulting playlist and mp3s to the syncthing folder for use by android device
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    QString syncthingPathstr = QDir::homePath() + "/Sync/"; // Set syncthing path
    // First, remove all existing mp3s from syncthing directory
    removeMP3sSyncthing();
    std::ifstream linuxfilepath;
    linuxfilepath.open (appDataPathstr.toStdString()+"/cleanedplaylist.txt");
    if (linuxfilepath.is_open()) {linuxfilepath.close();}
    else {std::cout << "syncPlaylistWithSyncthing: Error opening cleanedplaylist.txt file ." << std::endl;}
    std::string playlist = appDataPathstr.toStdString()+"/cleanedplaylist.txt"; // now we can use it as input file
    std::ifstream readlist(playlist);
    std::ofstream ofs1; //open the cleanedplaylist.m3u8 file for writing.
    ofs1.open(syncthingPathstr.toStdString()+"syncplaylist.m3u8"); // open new m3u8 file to syncthing location
    std::string str1;
    std::string findstring("/mnt/vboxfiles/music/*/*/");
    std::string replacestring("/storage/emulated/0/Download/");
    std::string androidstring("");
    std::string filenameonly("");
    std::string replacestring2("");
    int findstringlength2;
    std::string newdest;
    QString sourcepath;
    while(getline(readlist, str1)){ // Start while loop, change playlist path to "/storage/emulated/0/Download/"
        sourcepath = QString::fromUtf8(str1.c_str()); // Save str1 (source path) to QString
        filenameonly = str1;
        androidstring = str1; // String to store android file path
        findstringlength2 = positionOfXthDelimChar(filenameonly, 5);
        filenameonly.replace(0,findstringlength2+2,replacestring2); // Get filename only and save as str1
        newdest = syncthingPathstr.toStdString()+filenameonly; // Builds filepath for syncthing
        QFile::copy(sourcepath, QString::fromUtf8(newdest.c_str())); // Copy mp3 from souce to syncthing destination
        int findstringlength = positionOfXthDelimChar(androidstring, 5); // find position of sixth / delimiter
        androidstring.replace(0,findstringlength+2,replacestring); // replace string with android path
        ofs1 << androidstring << "\n"; // write revised path to output playlist file
    }
    ofs1.close();
    readlist.close();
}

void testid3tag(){
    //ShowKnownFrameInfo();
    ID3_Tag myTag;
    myTag.Link("/home/lpc123/Downloads/07_-_the_fray_-_enough_for_now.mp3",ID3TT_ID3V2 | ID3TT_APPENDED);
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
    if ( (frame = myTag.Find ( ID3FID_TRACKNUM )) )
   {
       char track[ 1024 ];
       frame->Field ( ID3FN_TEXT ).Get ( track, 1024 );
       std::cout << "Track: " << track << std::endl;
   }
    if ( (frame = myTag.Find ( ID3FID_ALBUM )) )
    {
        char album[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( album, 1024 );
        std::cout << "Album: " << album << std::endl;
    }
    if ( (frame = myTag.Find ( ID3FID_YEAR )) )
    {
        char year[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( year, 1024 );
        std::cout << "Year: " << year << std::endl;
    }
    if ( (frame = myTag.Find ( ID3FID_CONTENTGROUP )) )
    {
        char ratingcode[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( ratingcode, 1024 );
        std::cout << "Rating Code (Work): " << ratingcode << std::endl;
    }
    if ( (frame = myTag.Find ( ID3FID_USERTEXT )) )
    {
        char usertext1[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( usertext1, 1024 );
        std::cout << "usertext1: " << usertext1 << std::endl;
    }
    if ( (frame = myTag.Find ( ID3FID_USERTEXT )) )
    {
        char usertext2[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( usertext2, 1024 );
        std::cout << "usertext2: " << usertext2 << std::endl;
    }
    if ( (frame = myTag.Find ( ID3FID_USERTEXT )) )
    {
        char usertext3[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( usertext3, 1024 );
        std::cout << "usertext3: " << usertext3 << std::endl;
    }
    if ( (frame = myTag.Find ( ID3FID_MOOD )) )
    {
        char usertext4[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( usertext4, 1024 );
        std::cout << "usertext4: " << usertext4 << std::endl;
    }
     if ( (frame = myTag.Find ( ID3FID_COMMENT )) )
    {
        char comment[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( comment, 1024 );
        std::cout << "Comment: " << comment << std::endl;
    }
    else
        std::cout<<"not found\n";
}

void getLastPlayedDates(QString &s_androidpathname){
    // Gets lastplayed history from AIMP log and save it to lastplayeddates.txt
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/";
    std::fstream debuglog;
    debuglog.open (s_androidpathname.toStdString() + Constants::kAIMPLogPath);
    if (debuglog.is_open()) {debuglog.close();}
    else {std::cout << "getLastPlayedDates: Error opening debuglog.txt file." << std::endl;}
    std::string debuglogfile = s_androidpathname.toStdString() + Constants::kAIMPLogPath;
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
    std::string albumentry{""};
    int artistline {0};
    int songline ={0};
    int albumline ={0};
    std::string str2{""};
    std::string str3{""};
    std::vector<std::string>combinedvect;
    std::vector<std::string>finalvect;
    std::vector<std::string>outputvect;
    std::string str4;
    while (std::getline(debug, str1)) {
        std::size_t found = str1.find(lastplayedmarker);
        // Beginning of first section
        if (found!=std::string::npos) {  // found!= (not found) means the function did not return string::npos,
            // meaning a match was FOUND for the current line; Next get the date from that same line
            //define start and end positions for date string, then save date string
            str2 = str1;
            std::size_t foundstart = str2.find(begpos);
            std::size_t foundend1 = str2.find(endpos1);
            std::size_t foundend2 = str2.find(endpos2);
            std::size_t foundend = 0;
            if (foundstart!=std::string::npos)
                if (foundend1!=std::string::npos){
                    foundend = foundend1;
                }
            if (foundend2!=std::string::npos){
                foundend = foundend2;
            }
            if (str3 == ""){
                str3 = str2.substr (7,foundend-5);
                double sqlcon;
                sqlcon = logdateconversion(str3);
                collecteddate = std::__cxx11::to_string(sqlcon); // Store the string for date found
            }
            if (str3 != ""){
                artistline = linecount + 1; // Set the line number for artist variable after lastplayed date found
                albumline = linecount + 3;  // Set the line number for song title after lastplayed date found
                songline = linecount + 4;  // Set the line number for song title after lastplayed date found
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
                // Remove certain special characters from the artistentry string before writing line to lastplayeddates.txt
                std::string specchars = "\?@&()#\"+*!;"; /// Identify special characters to remove
                artistentry.erase(remove_if(artistentry.begin(), artistentry.end(),
                                            [&specchars](const char& c) {
                    return specchars.find(c) != std::string::npos;
                }),
                                  artistentry.end());
            }
            if ((linecount == albumline) && (collecteddate != "")){
                // get song string if it is two lines after lastplayed date found
                std::size_t strlength = str1.length();
                albumentry = str1.substr (6,strlength-6);
                // Remove certain special characters from the songentry string before writing line to lastplayeddates.txt
                std::string specchars = "\?@&()#\"+*!;"; /// Identify special characters to remove
                albumentry.erase(remove_if(albumentry.begin(), albumentry.end(),
                                          [&specchars](const char& c) {
                    return specchars.find(c) != std::string::npos;
                }),
                                albumentry.end());
            }
            if ((linecount == songline) && (collecteddate != "")){
                // get song string if it is two lines after lastplayed date found
                std::size_t strlength = str1.length();
                songentry = str1.substr (6,strlength-6);
                // Remove certain special characters from the songentry string before writing line to lastplayeddates.txt
                std::string specchars = "\?@&()#\"+*!;"; /// Identify special characters to remove
                songentry.erase(remove_if(songentry.begin(), songentry.end(),
                                          [&specchars](const char& c) {
                    return specchars.find(c) != std::string::npos;
                }),
                                songentry.end());
            }
            if ((linecount == songline +1) && (collecteddate != "")){
                // If the line after completing assignment of the 3 variables: date, artist and song, save all three into one csv line
                // to export or push into vector, then reset variables and continue going through the log
                lastplayedentry = artistentry + "," + albumentry+ "," + songentry+ "," +collecteddate;
                //std::cout << "lastplayedentry string is: " << lastplayedentry << '\n';
                combinedvect.push_back(lastplayedentry);

                //dateslist << lastplayedentry<<"\n";

                collecteddate = "";
                artistline = 0;
                albumline = 0;
                songline = 0;
                str3 = "";
            }
            ++linecount;
        }
    }
    debug.close();
    std::sort (combinedvect.begin(), combinedvect.end(), std::greater<>()); // Do a reverse sort to put newer dates first
    std::map<std::string, std::string> map; // map used to remove duplicates
    for(auto& el: combinedvect){
        auto it = el.find_last_of(',');           // find last ","
        auto key = el.substr(0, it);              // extract the key
        auto value = std::string(el.substr(it+1));  // extract the last value
        // if it does not exist already, or if it exists and has a value greater than the one inserted:
        if(map.find(key) == map.end() || (map.find(key) != map.end() && map[key] < value))
            map[key] = value; // change the value
    }
    for(auto& [k, combinedvect]: map){
        finalvect.push_back(k+","+combinedvect+"\n"); // Push back unique entries to new vector finalvect
    }
    std::string selectedLPLArtistToken; // Artist variable from lastplayeddates.txt
    std::string selectedLPLTitleToken; // Title variable from lastplayeddates.txt
    std::string selectedLPLAlbumToken; // Title variable from lastplayeddates.txt
    std::string selectedLPLSQLDateToken; // SQL Date variable from lastplayeddates.txt
    for (auto & d : finalvect){ // Iterate through finalvect for each line d
        std::stringstream s_streamd(d); //create string stream with line d
        std::vector<std::string> resulttemp; // create temp vector resulttemp to store tokens
        int tokenCount{0}; //token count is the number of delimiter characters within str
        while(s_streamd.good()) { // Iterate the string and parse tokens
            std::string token;
            while (std::getline(s_streamd, token, ','))
            {
                if (tokenCount == 0) {selectedLPLArtistToken = token;}
                if (tokenCount == 1) {selectedLPLAlbumToken = token;}
                if (tokenCount == 2) {selectedLPLTitleToken = token;}
                if (tokenCount == 3) {selectedLPLSQLDateToken = token;}
                ++ tokenCount;
            }
        }
        //Send all tokens to the new output vector with the date token placed at the front
        str4 = (selectedLPLSQLDateToken+","+selectedLPLArtistToken+","+selectedLPLAlbumToken+","+ selectedLPLTitleToken);
        outputvect.push_back(str4);
        resulttemp.shrink_to_fit();
    }
    std::sort (outputvect.begin(), outputvect.end()); // Now, sort the output vector by date
    // Now, output order to move the date back to the last element when writing file
    for (auto & j : outputvect) {// Iterate through the vector for each line j
        std::stringstream s_stream(j); //create string stream with line j
        std::vector<std::string> result; // create temp vector result
        std::string datetemp;
        std::string resttemp;
        while(s_stream.good()) {
            std::string substr;
            getline(s_stream, substr, ','); //get token (substr) delimited by comma
            result.push_back(substr); // populate result vector with tokens
        }
        for(unsigned long i = 0; i<result.size(); i++) {    // now reorder tokens from result with date in correct position
            datetemp = result.at(0);
            resttemp = result.at(1)+","+result.at(2)+","+result.at(3);
        }
        dateslist << resttemp <<","<< datetemp; // Write reordered tokens to file
        result.shrink_to_fit();
    }
    combinedvect.shrink_to_fit();
    finalvect.shrink_to_fit();
    outputvect.shrink_to_fit();
    dateslist.close();
}

void updateCleanLibDates(){
    // Uses lastplayeddates.txt to update last played dates in cleanlib.dsv for songs played and logged by AIMP
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/";
    QString tempFileStr1 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/cleanlib.dsv";
    QString tempFileStr2 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/cleanlib2.dsv";
    QFile::copy(tempFileStr1,tempFileStr2);
    // Create vector for new lastplayed dates
    StringVector2D lastplayedvec = readCSV(appDataPathstr.toStdString()+"lastplayeddates.txt"); // open "lastplayeddates.txt" as 2D vector lastplayedvec
    lastplayedvec.reserve(1000);
    std::string selectedArtistToken; // Artist variable from lastplayedvec
    std::string selectedTitleToken; // Title variable from lastplayedvec
    std::string selectedAlbumToken; // Title variable from lastplayedvec
    std::string selectedSQLDateToken; // SQL Date variable from lastplayedvec
    std::string selectedLibArtistToken; // Artist variable from cleanlib.dsv
    std::string selectedLibTitleToken; // Title variable from cleanlib.dsv
    std::string selectedLibAlbumToken; // Title variable from cleanlib.dsv
    std::string selectedLibSQLDateToken; // SQL Date variable from cleanlib.dsv
    // Open cleanlib.dsv as read file
    std::ifstream cleanlib;  // First ensure cleanlib.dsv is ready to open
    cleanlib.open (appDataPathstr.toStdString()+"/cleanlib2.dsv");
    if (cleanlib.is_open()) {cleanlib.close();}
    else {std::cout << "updateCleanLibDates: Error opening cleanlib2.dsv file." << std::endl;}
    std::string cleanlibSongsTable = appDataPathstr.toStdString()+"/cleanlib2.dsv";    // Now we can use it as input file
    std::ifstream SongsTable(cleanlibSongsTable);    // Open cleanlib.dsv as ifstream
    if (!SongsTable.is_open())
    {
        std::cout << "updateCleanLibDates: Error opening SongsTable." << std::endl;
        std::exit(EXIT_FAILURE); // Otherwise, quit
    }
    std::string str; // Create ostream file to update cleanLib
    std::ofstream outf(appDataPathstr.toStdString()+"/cleanlib.dsv"); // output file for writing revised lastplayed dates
    // Loop through cleanlib and find matches in lastplayed vector; for matches revised string with new SQL date and push to new file

    //  Outer loop: iterate through rows of SongsTable
    while (std::getline(SongsTable, str)) {   // Outer loop: iterate through rows of primary songs table
        // Declare variables applicable to all rows
        std::istringstream iss(str);
        std::string token;
        // Create a vector to parse each line by carat and do processing
        // Vector of string to save tokens
        std::vector<std::string> tokens;
        std::stringstream check1(str);// stringstream for parsing carat delimiter
        std::string intermediate; // intermediate value for parsing carat delimiter
        // Open tokens vector to tokenize current string using carat '^' delimiter
        while(getline(check1, intermediate, '^')) // Inner loop: iterate through tokens of string using tokens vector
        {
            tokens.push_back(intermediate);
        }
        selectedLibArtistToken = tokens[Constants::kColumn1];
        selectedLibAlbumToken = tokens[Constants::kColumn3];
        selectedLibTitleToken = tokens[Constants::kColumn7];
        selectedLibSQLDateToken = tokens[Constants::kColumn17];
        for(auto & i : lastplayedvec){ // read each row element from the lastplayedvec into variables for comparison with Lib tokens
            selectedArtistToken = i[Constants::kColumn0];
            selectedAlbumToken = i[Constants::kColumn1];
            selectedTitleToken = i[Constants::kColumn2];
            selectedSQLDateToken = i[Constants::kColumn3];
            // Match Artist and title in cleanLib from lastplayedvec and change SQL date for each if log date is newer
            if ((selectedArtistToken == selectedLibArtistToken) && (selectedAlbumToken == selectedLibAlbumToken)
                    && (selectedTitleToken == selectedLibTitleToken) && (std::stod(selectedSQLDateToken) > std::stod(selectedLibSQLDateToken))){
                tokens.at(Constants::kColumn17) = selectedSQLDateToken;
                str = getChgdDSVStr(tokens,str); // recompile str with changed token
                continue;
            }
        }
        outf << str << "\n"; // The string is valid, write to clean file
        tokens.shrink_to_fit();
    }
    SongsTable.close();
    outf.close();
    lastplayedvec.shrink_to_fit();
    removeAppData("cleanlib2.dsv"); // Remove cleanlib2.dsv
}

void removeLinuxPlaylistFile(){
    const std::string linuxplpathdirname = getenv("HOME");
    std::string plfile(linuxplpathdirname + "/cleanedplaylist.m3u");
    remove (plfile.c_str());// remove old exported playlist from home directory
}

void syncAudaciousLog(){
    // Concatenate Audacious log with a copy of lastplayeddates.txt, sort it by date, then
    // remove duplicate entries that have earlier dates
    // Uses lastplayeddates.txt to update last played dates in cleanlib.dsv for songs played and logged by AIMP
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/";
    QString tempFileStr1a = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/lastplayeddates.txt";
    QString tempFileStr1b = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/lastplayeddates2.txt";
    QString tempFileStr2 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/audacioushist.log";
    std::ifstream SongsTable1(tempFileStr1a.toStdString());    // Open lastplayeddates.txt as ifstream
    std::ifstream SongsTable2(tempFileStr2.toStdString());    // Open audacioushist.log.txt as ifstream
    std::ofstream combined_file(appDataPathstr.toStdString()+"/lastplayeddates2.txt"); // Create output file for combined
    combined_file << SongsTable1.rdbuf() << SongsTable2.rdbuf(); // Combine AIMP and Audacious logs as lastplayeddates2.txt
    SongsTable1.close();
    SongsTable2.close();
    combined_file.close();
    // Next, open and read combined file into a vector. Sort it by date, remove dups, then rewrite lastplayeddates.txt
    std::vector<std::string>combinedvect;
    std::vector<std::string>finalvect;
    std::vector<std::string>outputvect;
    std::string str;
    std::string str4;
    // Open lastplayeddates2.txt as read file
    std::ifstream LPLlib;  // First ensure lastplayeddates2.txt is ready to open
    LPLlib.open (appDataPathstr.toStdString()+"/lastplayeddates2.txt");
    if (LPLlib.is_open()) {LPLlib.close();}
    else {std::cout << "syncAudaciousLog: Error opening lastplayeddates2.txt file." << std::endl;}
    std::string LPLlibSongsTable = appDataPathstr.toStdString()+"/lastplayeddates2.txt";    // Now we can use it as input file
    std::ifstream SongsTable3(LPLlibSongsTable);    // Open lastplayeddates2.txt as ifstream
    if (!SongsTable3.is_open())
    {
        std::cout << "syncAudaciousLog: Error opening SongsTable." << std::endl;
        std::exit(EXIT_FAILURE); // Otherwise, quit
    }
    // Create ostream file to replace lastplayeddates.txt
    std::ofstream outf4(appDataPathstr.toStdString()+"/lastplayeddates.txt");
    //  Iterate through rows of SongsTable3 (lastplayeddates2.txt)
    while (std::getline(SongsTable3, str)) {
        // Clean up special charaters that came from Audacious entries
        std::string specchars = "\?@&()#\"+*!;"; /// Identify special characters to remove
        str.erase(remove_if(str.begin(), str.end(),
                                  [&specchars](const char& c) {
            return specchars.find(c) != std::string::npos;
        }),
                        str.end());
        combinedvect.push_back(str);
    }
    std::sort (combinedvect.begin(), combinedvect.end(), std::greater<>()); // Do a reverse sort to put newer dates first
    std::map<std::string, std::string> map; // map used to remove duplicates
    for(auto& el: combinedvect){
        auto it = el.find_last_of(',');           // find last ","
        auto key = el.substr(0, it);              // extract the key
        auto value = std::string(el.substr(it+1));  // extract the last value
        // if it does not exist already, or if it exists and has a value greater than the one inserted:
        if(map.find(key) == map.end() || (map.find(key) != map.end() && map[key] < value))
            map[key] = value; // change the value
    }
    for(auto& [k, combinedvect]: map){
        finalvect.push_back(k+","+combinedvect+"\n"); // Push back unique entries to new vector finalvect
    }
    std::string selectedLPLArtistToken; // Artist variable from lastplayeddates.txt
    std::string selectedLPLTitleToken; // Title variable from lastplayeddates.txt
    std::string selectedLPLAlbumToken; // Title variable from lastplayeddates.txt
    std::string selectedLPLSQLDateToken; // SQL Date variable from lastplayeddates.txt

    for (auto & d : finalvect){ // Iterate through finalvect for each line d
        std::stringstream s_streamd(d); //create string stream with line d
        std::vector<std::string> resulttemp; // create temp vector resulttemp to store tokens
        int tokenCount{0}; //token count is the number of delimiter characters within str
        while(s_streamd.good()) { // Iterate the string and parse tokens
            std::string token;
            while (std::getline(s_streamd, token, ','))
            {
                if (tokenCount == 0) {selectedLPLArtistToken = token;}
                if (tokenCount == 1) {selectedLPLAlbumToken = token;}
                if (tokenCount == 2) {selectedLPLTitleToken = token;}
                if (tokenCount == 3) {selectedLPLSQLDateToken = token;}
                ++ tokenCount;
            }
        }
        //Send all tokens to the new output vector with the date token placed at the front
        str4 = (selectedLPLSQLDateToken+","+selectedLPLArtistToken+","+selectedLPLAlbumToken+","+ selectedLPLTitleToken);
        outputvect.push_back(str4);
        resulttemp.shrink_to_fit();
    }
    std::sort (outputvect.begin(), outputvect.end()); // Now, sort the output vector by date
    // Now, output order to move the date back to the last element when writing file
    for (auto & j : outputvect) {// Iterate through the vector for each line j
        std::stringstream s_stream(j); //create string stream with line j
        std::vector<std::string> result; // create temp vector result
        std::string datetemp;
        std::string resttemp;
        while(s_stream.good()) {
            std::string substr;
            getline(s_stream, substr, ','); //get token (substr) delimited by comma
            result.push_back(substr); // populate result vector with tokens
        }
        for(unsigned long i = 0; i<result.size(); i++) {    // now reorder tokens from result with date in correct position
            datetemp = result.at(0);
            resttemp = result.at(1)+","+result.at(2)+","+result.at(3);
        }
        outf4 << resttemp <<","<< datetemp; // Write reordered tokens to file
        result.shrink_to_fit();
    }
    outf4.close();
    combinedvect.shrink_to_fit();
    finalvect.shrink_to_fit();
    outputvect.shrink_to_fit();
    removeAppData("lastplayeddates2.txt"); // Remove lastplayeddates2.txt
}
