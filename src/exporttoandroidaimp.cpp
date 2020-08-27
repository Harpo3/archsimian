#include <QMessageBox>
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
#include <id3/misc_support.h>
#include <taglib/tag.h>
#include <sys/stat.h>

bool doesFileExist3 (const std::string& name) {
    struct stat buffer{};
    return (stat (name.c_str(), &buffer) == 0);
}

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
    std::string selectedLibArtistToken{" "};
    std::string selectedLibTitleToken{" "};
    std::string selectedLibAlbumToken{" "};
    std::string bandToken;
    std::string titleToken;
    std::string albumToken;
    std::string ratingCode{" "};
    std::string ratingToken;

    ID3_Tag activeTag;
    activeTag.Link("/mnt/vboxfiles/music/smash_mouth/astro_lounge/09_-_smash_mouth_-_then_the_morning_comes.mp3",ID3TT_ID3V2 | ID3TT_APPENDED);

    ID3_Frame *frame;
    if ( (frame = activeTag.Find ( ID3FID_LEADARTIST )) )
    {
        char band[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( band, 1024 );
        std::cout << "band: " << band << std::endl;
        bandToken = band;
    }
    if ( (frame = activeTag.Find ( ID3FID_TITLE )) )
    {
        char title[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( title, 1024 );
        titleToken = title;
        std::cout << "title: " << title << std::endl;
    }
    if ( (frame = activeTag.Find ( ID3FID_ALBUM )) )
    {
        char album[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( album, 1024 );
        std::cout << "album: " << album << std::endl;
        albumToken = album;
    }
    if ( (frame = activeTag.Find ( ID3FID_POPULARIMETER )) )
    {
        char *sEmail = ID3_GetString(frame, ID3FN_EMAIL);
        size_t nRating = frame->GetField(ID3FN_RATING)->Get();
        std::cout <<"popm: " << nRating << std::endl;
        delete [] sEmail;
    }
    if ( (frame = activeTag.Find ( ID3FID_COMMENT)) ) // Comment frame (needed to iterate to the next frame)
    {
        char  *sText = ID3_GetString(frame, ID3FN_TEXT);
        char  *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
        char *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
        delete [] sText;
        delete [] sDesc;
        delete [] sLang;
    }
    if ( (frame = activeTag.Find ( ID3FID_COMMENT)) ) // MusicMatch Frame (needed to iterate to the next frame)
    {
        char  *sText = ID3_GetString(frame, ID3FN_TEXT);
        char  *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
        char *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
        delete [] sText;
        delete [] sDesc;
        delete [] sLang;
    }
    if ( (frame = activeTag.Find ( ID3FID_COMMENT)) ) // ArchSimian artist grouping name
    {
        char  *sText = ID3_GetString(frame, ID3FN_TEXT);
        char  *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
        char *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
        std::cout << "Artist Grouping: "
                  << sText <<" for the custome description: "<<sDesc << std::endl;
        delete [] sText;
        delete [] sDesc;
        delete [] sLang;
    }
    if ( (frame = activeTag.Find ( ID3FID_CONTENTGROUP )) )
    {
        char ratingcode[ 1024 ];
        frame->Field ( ID3FN_TEXT ).Get ( ratingcode, 1024 );
        std::cout << "ratingcode: " << ratingcode << std::endl;
        ratingToken = ratingcode;
    }
    std::cout << "ratingCode is: " << ratingCode << ". ratingToken is: "<< ratingToken<<". bandToken/titleToken is: " << bandToken <<" - "<<titleToken<< std::endl;
    if ( (bandToken == selectedLibArtistToken) && (titleToken == selectedLibTitleToken) && (albumToken == selectedLibAlbumToken)) {
        if ( (bandToken == selectedLibArtistToken) && (titleToken == selectedLibTitleToken) && (albumToken == selectedLibAlbumToken)) {
            if (ratingCode != ratingToken)               {
                std::cout << "ratingCode: " << ratingCode << " does NOT match ratingToken: "<< ratingToken<<" for: " << selectedLibArtistToken <<" - "<<selectedLibTitleToken<< std::endl;
            }
            if (ratingCode == ratingToken)               {
                std::cout << "ratingCode: " << ratingCode << " matches ratingToken: "<< ratingToken<<" for: " << selectedLibArtistToken <<" - "<<selectedLibTitleToken<< std::endl;
            }
        }
        else
            std::cout<<"not found\n";
    }
}

void getLastPlayedDates(QString s_androidpathname){
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
    bool matchinfile{0};
    while (std::getline(debug, str1)) {
        std::size_t found = str1.find(lastplayedmarker);
        // Beginning of first section
        if (found!=std::string::npos) {  // found!= (not found) means the function did not return string::npos,
            // meaning a match was FOUND for the current line; Next get the date from that same line
            //define start and end positions for date string, then save date string
            matchinfile = true;
            if (matchinfile == true){
                if(Constants::kVerbose){std::cout << "getLastPlayedDates: At least one entry found."<< std::endl;}
            }
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
                                         // meaning a match was NOT FOUND for the current line; Next,
            if ((linecount == artistline) && (collecteddate != "")){
                // Get artist string if it is two lines after lastplayed date found
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
                // Get album string if it is two lines after lastplayed date found
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
                // Get song string if it is two lines after lastplayed date found
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
                combinedvect.push_back(lastplayedentry);
                collecteddate = "";
                artistline = 0;
                albumline = 0;
                songline = 0;
                str3 = "";
            }
            ++linecount;
        }
    }
    if (matchinfile == false){ // Log has no entries with completed songs
        if (Constants::kVerbose){std::cout << "getLastPlayedDates: Terminating function. No entries found."<< std::endl;}
        dateslist.close();
        debug.close();
        combinedvect.shrink_to_fit();
        return ;
    }
    debug.close();
    std::sort (combinedvect.begin(), combinedvect.end(), std::greater<>()); // Do a reverse sort to put newer dates first
    std::map<std::string, std::string> map; // Map used to remove duplicates
    for(auto& el: combinedvect){
        auto it = el.find_last_of(',');           // Find last ","
        auto key = el.substr(0, it);              // Extract the key
        auto value = std::string(el.substr(it+1));  // Extract the last value
        // If it does not exist already, or if it exists and has a value greater than the one inserted,
        if(map.find(key) == map.end() || (map.find(key) != map.end() && map[key] < value))
            map[key] = value; // change the value.
    }
    for(auto& [k, combinedvect]: map){
        finalvect.push_back(k+","+combinedvect+"\n"); // Push back unique entries to new vector finalvect
    }
    std::string selectedLPLArtistToken; // Artist variable from lastplayeddates.txt
    std::string selectedLPLTitleToken; // Title variable from lastplayeddates.txt
    std::string selectedLPLAlbumToken; // Title variable from lastplayeddates.txt
    std::string selectedLPLSQLDateToken; // SQL Date variable from lastplayeddates.txt
    for (auto & d : finalvect){ // Iterate through finalvect for each line d
        std::stringstream s_streamd(d); // Create string stream with line d
        std::vector<std::string> resulttemp; // Create temp vector resulttemp to store tokens
        int tokenCount{0}; // Token count is the number of delimiter characters within str
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
        // Send all tokens to the new output vector with the date token placed at the front
        str4 = (selectedLPLSQLDateToken+","+selectedLPLArtistToken+","+selectedLPLAlbumToken+","+ selectedLPLTitleToken);
        outputvect.push_back(str4);
        resulttemp.shrink_to_fit();
    }
    std::sort (outputvect.begin(), outputvect.end()); // Sort the output vector by date
    // Output order to move the date back to the last element when writing file
    for (auto & j : outputvect) { // Iterate through the vector for each line j
        std::stringstream s_stream(j); //Create string stream with line j
        std::vector<std::string> result; // Create temp vector result
        std::string datetemp;
        std::string resttemp;
        while(s_stream.good()) {
            std::string substr;
            getline(s_stream, substr, ','); // Get token (substr) delimited by comma
            result.push_back(substr); // Populate result vector with tokens
        }
        for(unsigned long i = 0; i<result.size(); i++) { // Reorder tokens from result with date in correct position
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
    return ;
}

void updateCleanLibDates(){
    // Uses lastplayeddates.txt to update last played dates in cleanlib.dsv for songs played and logged by AIMP
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/";
    QString tempFileStr1 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/cleanlib.dsv";
    QString tempFileStr2 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/cleanlib2.dsv";
    QFile::copy(tempFileStr1,tempFileStr2);
    // Open log for reporting changes to UI
    std::ofstream ofs; // Open syncdisplay.txt for writing with the truncate option to delete the content.
    ofs.open(appDataPathstr.toStdString()+"/syncdisplay.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    std::ofstream lastplayedupdate(appDataPathstr.toStdString()+"/syncdisplay.txt",std::ios::app); // Output in append mode
    lastplayedupdate << "Last played entries updated to the ArchSimian database: "<< '\n';
    // Create vector for new lastplayed dates
    StringVector2D lastplayedvec = readCSV(appDataPathstr.toStdString()+"lastplayeddates.txt"); // Open as 2D vector lastplayedvec
    lastplayedvec.reserve(10000);
    std::string selectedArtistToken; // Artist variable from lastplayedvec
    std::string selectedTitleToken; // Title variable from lastplayedvec
    std::string selectedAlbumToken; // Title variable from lastplayedvec
    std::string selectedSQLDateToken; // SQL Date variable from lastplayedvec
    std::string selectedLibArtistToken; // Artist variable from cleanlib.dsv
    std::string selectedLibTitleToken; // Title variable from cleanlib.dsv
    std::string selectedLibAlbumToken; // Title variable from cleanlib.dsv
    std::string selectedLibSQLDateToken; // SQL Date variable from cleanlib.dsv
    try { // Operation replaces cleanlib.dsv; need to protect data in event of a fatal error
        // Open cleanlib2.dsv as read file
        std::ifstream cleanlib;  // First ensure cleanlib2.dsv is ready to open
        cleanlib.open (appDataPathstr.toStdString()+"/cleanlib2.dsv");
        if (cleanlib.is_open()) {cleanlib.close();}
        else {std::cout << "updateCleanLibDates: Error opening cleanlib2.dsv file." << std::endl;}
        std::string cleanlibSongsTable = appDataPathstr.toStdString()+"/cleanlib2.dsv";    // Use as input file
        std::ifstream SongsTable(cleanlibSongsTable);    // Open cleanlib2.dsv as ifstream
        if (!SongsTable.is_open())
        {
            std::cout << "updateCleanLibDates: Error opening SongsTable." << std::endl;
            std::exit(EXIT_FAILURE); // Otherwise, quit
        }
        std::string str; // Create ostream file to update cleanLib
        std::ofstream outf(appDataPathstr.toStdString()+"/cleanlib.dsv"); // Output file for writing revised lastplayed dates
        // Loop through cleanlib and find matches in lastplayed vector; for matches revised string with new SQL date and push to new file
        while (std::getline(SongsTable, str)) {   // Outer loop: iterate through rows of cleanlib
            std::istringstream iss(str);
            std::string token;
            std::vector<std::string> tokens;// Vector of string to parse each line by carat to save tokens
            std::stringstream check1(str);// Stringstream for parsing carat delimiter
            std::string intermediate; // Intermediate value for parsing carat delimiter
            // Open tokens vector to tokenize current string using carat '^' delimiter
            while(getline(check1, intermediate, '^'))
            {
                tokens.push_back(intermediate); // Inner loop: populate tokens vector
            }
            selectedLibArtistToken = tokens[Constants::kColumn1];
            selectedLibAlbumToken = tokens[Constants::kColumn3];
            selectedLibTitleToken = tokens[Constants::kColumn7];
            selectedLibSQLDateToken = tokens[Constants::kColumn17];
            for(auto & i : lastplayedvec){ // Assign row elements from lastplayedvec to variables to compare with Lib tokens
                selectedArtistToken = i[Constants::kColumn0];
                selectedAlbumToken = i[Constants::kColumn1];
                selectedTitleToken = i[Constants::kColumn2];
                selectedSQLDateToken = i[Constants::kColumn3];
                // Match Artist and title in cleanLib from lastplayedvec and change SQL date for each if log date is newer
                if ((selectedArtistToken == selectedLibArtistToken) && (selectedAlbumToken == selectedLibAlbumToken)
                        && (selectedTitleToken == selectedLibTitleToken) && (std::stod(selectedSQLDateToken) > std::stod(selectedLibSQLDateToken))){
                    tokens.at(Constants::kColumn17) = selectedSQLDateToken;
                    str = getChgdDSVStr(tokens,str); // Recompile str with changed token
                    lastplayedupdate << selectedLibArtistToken <<" - "<<selectedLibTitleToken<<"; SQL Date: "<<selectedSQLDateToken<< '\n';
                    continue;
                }
            }
            outf << str << "\n"; // The string is valid, write to clean file
            tokens.shrink_to_fit();
        }
        SongsTable.close();
        outf.close();
        lastplayedupdate.close();
        lastplayedvec.shrink_to_fit();
    }
    catch(const std::bad_alloc& exception) {
        std::cerr << "updateCleanLibDates: error detected: There was a problem writing to cleanlib.dsv. Replace with cleanlib2.dsv found in "
                     "/.local/share/archsimian/ and inspect file for errors." << exception.what();
        QMessageBox msgBox;
        QString msgboxtxt = "updateCleanLibDates: There was a problem writing to cleanlib.dsv. Replace with cleanlib2.dsv found in "
                            "/.local/share/archsimian/ and inspect file for errors.";
        msgBox.setText(msgboxtxt);
        msgBox.exec();
        qApp->quit(); //Exit program
    }
    removeAppData("cleanlib2.dsv"); // Remove cleanlib2.dsv
}

void updateChangedTagRatings(){
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/";
    QString tempFileStr1 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/cleanlib.dsv";
    QString tempFileStr2 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/cleanlib2.dsv";
    QFile::copy(tempFileStr1,tempFileStr2);
    std::string selectedLibsongPath{" "};
    std::string selectedLibpopmRating{" "};
    std::string selectedLibSQLDateToken{" "};
    std::string selectedLibratingCode{" "};
    std::string selectedLibArtistToken{" "};
    std::string selectedLibTitleToken{" "};
    std::string selectedLibAlbumToken{" "};
    std::string bandToken{" "};
    std::string titleToken{" "};
    std::string albumToken{" "};
    std::string ratingCode{" "};
    std::string ratingToken{" "};
    std::string popmToken{" "};
    std::string longstring{" "};
    // Open log for reporting changes to UI
    std::ofstream ofs; // Open the ratingupdate file for writing with the truncate option to delete the content.
    ofs.open(appDataPathstr.toStdString()+"/syncdisplay.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    std::ofstream ratingupdate(appDataPathstr.toStdString()+"/syncdisplay.txt",std::ios::app); // Open in append mode
    // Open cleanlib.dsv as read file
    std::ifstream cleanlib;  // Ensure cleanlib.dsv is ready to open
    cleanlib.open (appDataPathstr.toStdString()+"/cleanlib2.dsv");
    if (cleanlib.is_open()) {cleanlib.close();}
    else {std::cout << "updateCleanLibDates: Error opening cleanlib2.dsv file." << std::endl;}
    std::string cleanlibSongsTable = appDataPathstr.toStdString()+"/cleanlib2.dsv";    // Use as input file
    std::ifstream SongsTable(cleanlibSongsTable); // Open cleanlib.dsv as ifstream
    if (!SongsTable.is_open())
    {
        std::cout << "updateCleanLibDates: Error opening SongsTable." << std::endl;
        std::exit(EXIT_FAILURE); // Otherwise, quit
    }
    std::string str; // Create ostream file to update cleanLib
    std::ofstream outf(appDataPathstr.toStdString()+"/cleanlib.dsv"); // Output file for writing revised lastplayed dates
    std::getline(SongsTable, str); // Get column titles header string
    outf << str << "\n"; // Write column titles header string to first line of file
    // Loop through cleanlib and check each tag for changed ratings
    //  Outer loop: iterate through rows of SongsTable
    try { // Operation replaces cleanlib.dsv; need to protect data in event of a fatal error
    while (std::getline(SongsTable, str)) {   // Outer loop: iterate through rows of primary songs table
        // Declare variables applicable to all rows
        std::istringstream iss(str);
        // Create a vector to parse each line by carat and do processing
        std::vector<std::string> tokens2; // Vector of string to save tokens
        tokens2.reserve(50000);
        std::stringstream check1(str);// Stringstream for parsing carat delimiter
        std::string intermediate; // Intermediate value for parsing carat delimiter
        // Open tokens vector to tokenize current string using carat '^' delimiter
        while(getline(check1, intermediate, '^')) // Inner loop: iterate through tokens of string using tokens vector
        {
            tokens2.push_back(intermediate);
        }
        selectedLibArtistToken = tokens2[Constants::kColumn1];
        selectedLibAlbumToken = tokens2[Constants::kColumn3];
        selectedLibTitleToken = tokens2[Constants::kColumn7];
        selectedLibsongPath = tokens2[Constants::kColumn8];
        selectedLibpopmRating = tokens2[Constants::kColumn13];
        selectedLibSQLDateToken = tokens2[Constants::kColumn17];
        selectedLibratingCode = tokens2[Constants::kColumn29];
        longstring = "^"+tokens2[Constants::kColumn18]+"^"+tokens2[Constants::kColumn19]+"^"
                +tokens2[Constants::kColumn20]+"^"+tokens2[Constants::kColumn21]+"^"+tokens2[Constants::kColumn22]+"^"
                +tokens2[Constants::kColumn23]+"^"+tokens2[Constants::kColumn24]+"^"+tokens2[Constants::kColumn25]+"^"
                +tokens2[Constants::kColumn26]+"^"+tokens2[Constants::kColumn27]+"^"+tokens2[Constants::kColumn28];
        if (selectedLibratingCode == "0"){
            outf << str << "\n"; // The existing string is valid if unrated; if so, write unchanged string to cleanlib file
            continue;
        }
        ID3_Tag activeTag; // Get tag info using id3/tag.h
        activeTag.Link(selectedLibsongPath.c_str(),ID3TT_ID3V2 | ID3TT_APPENDED);
        ID3_Frame *frame;
        if ( (frame = activeTag.Find ( ID3FID_LEADARTIST )) )
        {
            char band[ 2048 ];
            frame->Field ( ID3FN_TEXT ).Get ( band, 2048 );
            bandToken = band;
        }
        if ( (frame = activeTag.Find ( ID3FID_TITLE )) )
        {
            char title[ 2048 ];
            frame->Field ( ID3FN_TEXT ).Get ( title, 2048 );
            titleToken = title;
        }
        if ( (frame = activeTag.Find ( ID3FID_ALBUM )) )
        {
            char album[ 2048 ];
            frame->Field ( ID3FN_TEXT ).Get ( album, 2048 );
            albumToken = album;
        }
        if ( (frame = activeTag.Find ( ID3FID_POPULARIMETER )) )
        {
            char *sEmail = ID3_GetString(frame, ID3FN_EMAIL);
            size_t nRating = frame->GetField(ID3FN_RATING)->Get();
            popmToken = std::to_string(nRating);
            delete [] sEmail;
        }
        if ( (frame = activeTag.Find ( ID3FID_COMMENT)) ) // Comment frame (needed to iterate to the next frame)
        {
            char  *sText = ID3_GetString(frame, ID3FN_TEXT);
            char  *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
            char *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
            delete [] sText;
            delete [] sDesc;
            delete [] sLang;
        }
        if ( (frame = activeTag.Find ( ID3FID_COMMENT)) ) // MusicMatch Frame (needed to iterate to the next frame)
        {
            char  *sText = ID3_GetString(frame, ID3FN_TEXT);
            char  *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
            char *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
            delete [] sText;
            delete [] sDesc;
            delete [] sLang;
        }
        if ( (frame = activeTag.Find ( ID3FID_COMMENT)) ) // ArchSimian artist grouping name
        {
            char  *sText = ID3_GetString(frame, ID3FN_TEXT);
            char  *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
            char *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
            delete [] sText;
            delete [] sDesc;
            delete [] sLang;
        }
        if ( (frame = activeTag.Find ( ID3FID_CONTENTGROUP )) )
        {
            char ratingcode[ 1024 ];
            frame->Field ( ID3FN_TEXT ).Get ( ratingcode, 1024 );
            ratingToken = ratingcode;
        }
        // Fix tag discrepancies for selectedLibArtistToken, titleToken, albumToken
        trim(bandToken);
        trim(titleToken);
        trim(albumToken);
        bandToken.erase(std::remove_if(bandToken.begin(), bandToken.end(), &IsSpecialChar), bandToken.end());
        titleToken.erase(std::remove_if(titleToken.begin(), titleToken.end(), &IsSpecialChar), titleToken.end());
        albumToken.erase(std::remove_if(albumToken.begin(), albumToken.end(), &IsSpecialChar), albumToken.end());

        if ( (bandToken == selectedLibArtistToken) && (titleToken == selectedLibTitleToken) && (albumToken == selectedLibAlbumToken)) {
            // If tag rating does not match cleanlib entry, change cleanlib variables to match before writing string
            if (selectedLibratingCode != ratingToken) {
                std::string tempoldrating = selectedLibratingCode;
                if (ratingToken!=""){selectedLibratingCode = ratingToken;}
                ratingupdate << "Rating changed from "<<tempoldrating<<" to "<<ratingToken << " for "<<
                                                                      selectedLibArtistToken <<" - "<<selectedLibTitleToken<< '\n';
                if (selectedLibArtistToken == "3"){selectedLibpopmRating = "100";} // Set MM4 code (40, 50, 60, etc) based on code or popm change
                if (selectedLibArtistToken == "4"){selectedLibpopmRating = "90";}
                if (selectedLibArtistToken == "5"){selectedLibpopmRating = "70";}
                if (selectedLibArtistToken == "6"){selectedLibpopmRating = "60";}
                if (selectedLibArtistToken == "7"){selectedLibpopmRating = "50";}
                if (selectedLibArtistToken == "8"){selectedLibpopmRating = "30";}
                str=tokens2.at(Constants::kColumn0)+"^"+selectedLibArtistToken+"^"+tokens2.at(Constants::kColumn2)+"^"+selectedLibAlbumToken+"^"
                        +tokens2.at(Constants::kColumn4)+"^"+tokens2.at(Constants::kColumn5)+"^"+tokens2.at(Constants::kColumn6)+"^"
                        +selectedLibTitleToken+"^"+selectedLibsongPath+"^"+tokens2.at(Constants::kColumn9)+"^"+tokens2.at(Constants::kColumn10)
                        +"^"+tokens2.at(Constants::kColumn11)+"^"+tokens2.at(Constants::kColumn12)+"^"+selectedLibpopmRating+"^"+
                        tokens2.at(Constants::kColumn14)+"^"+tokens2.at(Constants::kColumn15)+"^"+tokens2.at(Constants::kColumn16)
                        +"^"+selectedLibSQLDateToken+longstring+"^"+selectedLibratingCode;
                outf << str << "\n"; // The tag elements of artist, title, album match, and rating has been changed in cleanlib.
                bandToken="";
                titleToken="";
                albumToken="";
                ratingCode="";
                ratingToken="";
                popmToken="";
                continue;
            }
        }
        outf << str << "\n"; // One or more tag elements (artist, title, album) do NOT match cleanlib entry, but rating might match.
        bandToken="";
        titleToken="";
        albumToken="";
        ratingCode="";
        ratingToken="";
        popmToken="";
        tokens2.shrink_to_fit();
    }
    SongsTable.close();
    outf.close();
    ratingupdate.close();
    }
    catch(const std::bad_alloc& exception) {
            std::cerr << "updateChangedTagRatings: error detected: There was a problem writing to cleanlib.dsv. Replace with cleanlib2.dsv found in "
                         "/.local/share/archsimian/ and inspect file for errors." << exception.what();
            QMessageBox msgBox;
            QString msgboxtxt = "updateChangedTagRatings: Failed during attempt to process changed ratings. There was a problem writing to cleanlib.dsv. "
                                "Replace with cleanlib2.dsv found in /.local/share/archsimian/ and inspect file for errors.";
            msgBox.setText(msgboxtxt);
            msgBox.exec();
            qApp->quit(); //Exit program
        }
    removeAppData("cleanlib2.dsv"); // Remove cleanlib2.dsv
}

void removeLinuxPlaylistFile(){
    const std::string linuxplpathdirname = getenv("HOME");
    std::string plfile(linuxplpathdirname + "/cleanedplaylist.m3u");
    remove (plfile.c_str());// remove old exported playlist from home directory
}


void syncAudaciousLog(){
    QString tempFileStr1a = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/lastplayeddates.txt";
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/";
    //Check whether lastplayeddates currently has any data in it to see if the the AIMP log was empty of played entries
    QString DBfile = tempFileStr1a;
    QFile newFile(DBfile);
    newFile.open( QIODevice::WriteOnly|QIODevice::Append );
    if (newFile.pos() == 0) {
      // If the file is empty, use only the Audacious log to write lastplayeddates.txt
        if(Constants::kVerbose){std::cout << "syncAudaciousLog: AIMP log was empty."<< std::endl;}
        QString tempFileStr1 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/audacioushist.log";
        QString tempFileStr2 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/lastplayeddates2.txt";
        QFile::copy(tempFileStr1,tempFileStr2);
    } else {
      // If the file is not empty, combine the AIMP (lastplayeddates2.txt) and Audacious logs to write as lastplayeddates2.txt
        if(Constants::kVerbose){std::cout << "syncAudaciousLog: AIMP log was not empty."<< std::endl;}
        QString tempFileStr1b = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/lastplayeddates2.txt";
        QString tempFileStr2 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/audacioushist.log";
        std::ifstream SongsTable1(tempFileStr1a.toStdString());    // Open lastplayeddates.txt as ifstream
        std::ifstream SongsTable2(tempFileStr2.toStdString());    // Open audacioushist.log.txt as ifstream
        std::ofstream combined_file(appDataPathstr.toStdString()+"/lastplayeddates2.txt"); // Create output file for combined
        combined_file << SongsTable1.rdbuf() << SongsTable2.rdbuf(); // Combine AIMP and Audacious logs as lastplayeddates2.txt
        SongsTable1.close();
        SongsTable2.close();
        combined_file.close();
    }
    // Open and read combined file into a vector. Sort it by date, remove dups, then rewrite lastplayeddates.txt
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
    std::ofstream outf4(appDataPathstr.toStdString()+"/lastplayeddates.txt"); // Create ostream file to replace lastplayeddates.txt
    while (std::getline(SongsTable3, str)) { //  Iterate through rows of SongsTable3 (lastplayeddates2.txt)
        // Clean up special charaters that came from Audacious tag entries
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
    // Output order to move the date back to the last element when writing file
    for (auto & j : outputvect) { // Iterate through the vector for each line j
        std::stringstream s_stream(j); // Create string stream with line j
        std::vector<std::string> result; // Create temp vector result
        std::string datetemp;
        std::string resttemp;
        while(s_stream.good()) {
            std::string substr;
            getline(s_stream, substr, ','); // Get token (substr) delimited by comma
            result.push_back(substr); // Populate result vector with tokens
        }
        for(unsigned long i = 0; i<result.size(); i++) {    // Reorder tokens from result with date in correct position
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
    removeAppData("lastplayeddates2.txt");
}
