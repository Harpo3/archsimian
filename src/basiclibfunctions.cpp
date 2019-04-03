#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>
#include <QString>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include "constants.h"
#include "lastplayeddays.h"
#include "utilities.h"

inline bool doesFileExist (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

bool recentlyUpdated(const QString &s_mmBackupDBDir)
{
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    bool existResult{0};
    bool refreshNeededResult{0};
    std::string convertStd2 = appDataPathstr.toStdString()+"/cleanlib.dsv";
    existResult = doesFileExist(convertStd2);// See inline function at top
    if (Constants::verbose == true) std::cout << "recentlyUpdated(): doesFileExist() result for cleanlib.dsv is " << existResult << std::endl;
    if (existResult == 0) {refreshNeededResult = 1;}
    // If the lib file exists, Get the epoch date for the MM.DB file and see which file is older
    if (existResult == 1){
        std::string mmdbdir = s_mmBackupDBDir.toStdString();
        std::string mmpath = mmdbdir + "/MM.DB";
        struct stat stbuf1;
        stat(mmpath.c_str(), &stbuf1);
        localtime(&stbuf1.st_mtime); // or gmtime() depending on what you want        
        if (Constants::verbose == true) std::cout << "MM.DB is " << stbuf1.st_mtime << std::endl;        
        struct stat stbuf2; // Now get the date for the cleanlib.csv file
        std::string mmpath99 = appDataPathstr.toStdString()+"/cleanlib.dsv";
        stat(mmpath99.c_str(), &stbuf2);
        localtime(&stbuf2.st_mtime);        
        if (Constants::verbose == true) std::cout << "cleanlib.csv is " << stbuf2.st_mtime << std::endl;
        double dateResult = stbuf1.st_mtime - stbuf2.st_mtime;
        if (dateResult > 0) {
            refreshNeededResult = 1;
            if (Constants::verbose == true) std::cout << "MM.DB was recently backed up. Updating library and stats..." << std::endl;
        }
        // If the result is negative, then MM4 has not been updated since the program library was last refreshed. No update is necessary.
        // If positive, need to refresh all library data.
    }
    return refreshNeededResult;
}

// Function to create the library file using the exported songs table (libtable.dsv) of the MediMonkey database
void getLibrary(const QString &s_musiclibrarydirname)
{
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::ifstream filestr1;
    filestr1.open (appDataPathstr.toStdString()+"/libtable.dsv");
    if (filestr1.is_open()) {filestr1.close();}
    else {std::cout << "getLibrary: Error opening libtable.dsv file." << std::endl;}
    std::string databaseFile = appDataPathstr.toStdString()+"/libtable.dsv"; // now we can use it as a temporary input file
    std::ofstream outf(appDataPathstr.toStdString()+"/cleanlib.dsv"); // output file for writing clean track paths
    std::ifstream primarySongsTable(databaseFile);
    if (!primarySongsTable.is_open())
    {
        std::exit(EXIT_FAILURE);
    }
    std::string str;
    std::getline(primarySongsTable, str); //Get column titles header string
    outf << str << "\n"; // Write column titles header string to first line of file
    while (std::getline(primarySongsTable, str)) {   // Outer loop: iterate through rows of primary songs table
        // Declare variables applicable to all rows
        std::istringstream iss(str);
        std::string token;
        // Create a vector to parse each line by carat and do processing
        // Vector of string to save tokens
        std::vector<std::string> tokens;
        std::vector<std::string> dirPathTokens;//separate vector for parsing the directory path
        // stringstream class check1
        std::stringstream check1(str);// stringstream for parsing carat delimiter
        std::string intermediate; // intermediate value for parsing carat delimiter
        // Open tokens vector to tokenize current string using carat '^' delimiter
        while(getline(check1, intermediate, '^'))
        {
            tokens.push_back(intermediate);
        }
        // Within tokens vector, open dirPathTokens vector to tokenize dir path of tokens[8] using '\' as delimiter
        // then redefine tokens[8] with the corrected path
        std::string songPath1;
        songPath1 = tokens[8];        
        std::istringstream iss2(songPath1);
        std::stringstream check2(songPath1);// stringstream for parsing \ delimiter of dir path
        std::string intermediate2; // intermediate value for parsing \ delimiter of dir path
        while(getline(check2, intermediate2, '\\'))
        {
            dirPathTokens.push_back(intermediate2);
        }
        dirPathTokens.at(0) = s_musiclibrarydirname.toStdString();
        songPath1 = getChgdDirStr(dirPathTokens,songPath1,s_musiclibrarydirname);        
        int zeroCount = countBlankChars(songPath1); // run function to identify any paths that have blank spaces
        if (zeroCount > 0){
            QMessageBox msgBox;
            QString pathstr = QString::fromStdString(songPath1);
            msgBox.setText("ERROR: ArchSimian getLibrary Function - File path for: " + pathstr + " contains spaces. Use MediaMonkey to auto-organize"
                                                                                                 " and ensure this and all filenames do not have spaces. "
                                                                                                 "See README file. Exiting.");
            msgBox.exec();
            removeAppData ("libtable.dsv");
            removeAppData ("cleanlib.dsv");
            exit(1);
        }
        tokens.at(8) = songPath1;
        dirPathTokens.shrink_to_fit();
        //Adds a calculated rating code to Col 29 if Col 29 does not have a rating code already
        if (tokens[29] == "") {
            std::string newstr;
            if (tokens[13] == "100") newstr = "3";
            if ((tokens[13] == "90") || (tokens[13] == "80")) newstr = "4";
            if (tokens[13] == "70") newstr = "5";
            if (tokens[13] == "60") newstr = "6";
            if (tokens[13] == "50") newstr = "7";
            if ((tokens[13] == "30") || (tokens[13] == "40")) newstr = "8";
            if (tokens[13] == "20") newstr = "1";
            if (tokens[13] == "10") newstr = "0";
            if (tokens[13] == "0") newstr = "0";
            if (tokens[13] == "") newstr = "0";
            tokens.at(29) = newstr;
        }
        if ((tokens[13] != "0") && ((tokens[17] == "0.0")||(tokens[17] == "0"))){
            // generate a random lastplayed date if its current
            //  value is "0" unless track has a zero star rating
            // Process a function to generate a random date 30-500 days ago then save to a string
            double rndresult{0.0};
            int intconvert;
            std::string strrandom;
            rndresult = getNewRandomLPDate(rndresult);
            if (rndresult == 0.0)
            {std::cout << "getLibrary: Error obtaining random number." << std::endl;}
            intconvert = int (rndresult); // convert the random number to an integer
            strrandom = std::to_string(intconvert); // convert the integer to string
            tokens.at(17) = strrandom;
        }
        //Adds artist (without any spaces) to Col 19 if Col 19 does not have a custom value already
        if ((tokens[13] != "0") && (tokens[19] == "")) {
            tokens.at(19) = tokens[1];
            tokens.at(19) = removeSpaces(tokens[19]);
        }
        str = getChgdDSVStr(tokens,str);
        outf << str << "\n"; // The string is valid, write to clean file
        tokens.shrink_to_fit();
    }
    primarySongsTable.close();   // Close files opened for reading and writing
    outf.close();
}

// Function to collect and process music library data needed to calculate song placement in the playlist
void getDBStats(int *_srCode0TotTrackQty,int *_srCode0MsTotTime,int *_srCode1TotTrackQty,int *_srCode1MsTotTime,
                 int *_srCode3TotTrackQty,int *_srCode3MsTotTime,int *_srCode4TotTrackQty,int *_srCode4MsTotTime,
                 int *_srCode5TotTrackQty,int *_srCode5MsTotTime,int *_srCode6TotTrackQty,int *_srCode6MsTotTime,
                 int *_srCode7TotTrackQty,int *_srCode7MsTotTime,int *_srCode8TotTrackQty,int *_srCode8MsTotTime,
                 int *_sSQL10TotTimeListened,int *_sSQL10DayTracksTot,double *_sSQL20TotTimeListened,
                 int *_sSQL20DayTracksTot,double *_sSQL30TotTimeListened,int *_sSQL30DayTracksTot,double *_sSQL40TotTimeListened,
                 int *_sSQL40DayTracksTot,double *_sSQL50TotTimeListened,int *_sSQL50DayTracksTot,double *_sSQL60TotTimeListened,
                 int *_sSQL60DayTracksTot) {
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::ifstream filestr1;
    std::string combinedPath = appDataPathstr.toStdString()+"/cleanlib.dsv";
    filestr1.open (combinedPath);
    if (filestr1.is_open()) {filestr1.close();}
    else {std::cout << "getDBStats: Error opening cleanlib.dsv file." << std::endl;}
    std::string databaseFile = combinedPath; // now we can use it as input file
    std::ifstream primarySongsTable(databaseFile);
    double currDate = std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::system_clock::now().time_since_epoch()).count(); // This will go to lastplayed .cpp and .h
    // The conversion formula for epoch time to SQL time is: x = (x / 86400) + 25569  43441.4712847 43440.4712847
    double currSQLDate = (currDate / 86400) + 25569;        // This will go to lastplayed .cpp and .h
    if (!primarySongsTable.is_open())
    {
        std::exit(EXIT_FAILURE);
    }
    std::string str;
    int stringCount{0};
    while (std::getline(primarySongsTable, str)) {   // Outer loop: iterate through rows of primary songs table
        // Declare variables applicable to all rows
        std::istringstream iss(str);
        std::string strnew;
        std::string token;
        std::string tempTokenTrackTime;
        std::string tempTokenLastPlayedTime{"0.0"};
        int tokenCount{0};
        while (std::getline(iss, token, '^')) { // Inner loop: iterate through each column (token) of row
            // Store the time in milliseconds (col 12) in tempTokenTrackTime text variable
            if (tokenCount == 12) {tempTokenTrackTime = token;}
            // Store the lastplayed date string tempTokenLastPlayedTime text variable
            if (tokenCount == 17) {tempTokenLastPlayedTime = token;}
            // TOKEN PROCESSING - COL 17
            // Collect lastplayed stats to compute avg listening time
            double tempLastPlayedDate{0.0};
            std::string strrandom;
            if (tokenCount == 17) {
                tempLastPlayedDate = std::atof(token.c_str());
                if (tempLastPlayedDate > (currSQLDate - 60.9999))
                {
                    if (tempLastPlayedDate > (currSQLDate - 10.9999))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL10TotTimeListened = *_sSQL10TotTimeListened + timeint;
                        ++*_sSQL10DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 11) && (tempLastPlayedDate > (currSQLDate - 20.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL20TotTimeListened = *_sSQL20TotTimeListened + timeint;
                        ++*_sSQL20DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 21) && (tempLastPlayedDate > (currSQLDate - 30.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL30TotTimeListened = *_sSQL30TotTimeListened + timeint                                ;
                        ++*_sSQL30DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 31) && (tempLastPlayedDate > (currSQLDate - 40.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL40TotTimeListened = *_sSQL40TotTimeListened + timeint;
                        ++*_sSQL40DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 41) && (tempLastPlayedDate > (currSQLDate - 50.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL50TotTimeListened = *_sSQL50TotTimeListened + timeint;
                        ++*_sSQL50DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 51) && (tempLastPlayedDate > (currSQLDate - 60.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL60TotTimeListened = *_sSQL60TotTimeListened + timeint;
                        ++*_sSQL60DayTracksTot;}
                }
            }
            // TOKEN PROCESSING - COLS 12, 29
            // Then, check the current line for the GroupDesc (rating code, col 29), which is
            // then used to increment the song quantity and song time accordingly
            // Cols 12 and 29 are for song time and GroupDesc (rating code)
            // Collect statistical data for times and qtys of each rating category
            if (tokenCount == 29 && token == "0") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode0MsTotTime = *_srCode0MsTotTime + timeint;
                ++*_srCode0TotTrackQty;
            }
            if (tokenCount == 29 && token == "1") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode1MsTotTime = *_srCode1MsTotTime + timeint;
                ++*_srCode1TotTrackQty;
            }
            if (tokenCount == 29 && token == "3") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode3MsTotTime = *_srCode3MsTotTime + timeint;
                ++*_srCode3TotTrackQty;
            }
            if (tokenCount == 29 && token == "4") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode4MsTotTime = *_srCode4MsTotTime + timeint;
                ++*_srCode4TotTrackQty;
            }
            if (tokenCount == 29 && token == "5") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode5MsTotTime = *_srCode5MsTotTime + timeint;
                ++*_srCode5TotTrackQty;
            }
            if (tokenCount == 29 && token == "6") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode6MsTotTime = *_srCode6MsTotTime + timeint;
                ++*_srCode6TotTrackQty;
            }
            if (tokenCount == 29 && token == "7") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode7MsTotTime = *_srCode7MsTotTime + timeint;
                ++*_srCode7TotTrackQty;
            }
            if (tokenCount == 29 && token == "8") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode8MsTotTime = *_srCode8MsTotTime + timeint;
                ++*_srCode8TotTrackQty;
            }
            ++ tokenCount;
        }
        ++ stringCount;
    }
    primarySongsTable.close();   // Close files opened for reading and writing
}



// Function to create the file ratedabbr.txt which adds artist intervals, and which will then be used for track selection functions
void getSubset()
{
    if (Constants::verbose == true) std::cout << "Building the Archsimian database with artist intervals calculated....";
        QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();    
    std::ofstream ratedabbr(appDataPathstr.toStdString()+"/ratedabbr.txt"); // output file for subset table
    std::fstream filestrinterval;
    filestrinterval.open (appDataPathstr.toStdString()+"/cleanlib.dsv");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "Error opening cleanLibFile." << std::endl;}
    std::string ratedlibrary = appDataPathstr.toStdString()+"/cleanlib.dsv"; // now we can use it as input file
    std::ifstream primarySongsTable(ratedlibrary);
    if (!primarySongsTable.is_open()){
        std::cout << "Error opening cleanLibFile." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for cleanLibFile
    std::string str2; // store the string for playlistTable
    std::string str3; // store the string for artistsadjVec
    std::string str4; // store the string for vector parsing string of each artistadj line
    std::string selectedArtistToken; // Artist variable from
    std::string songPath;
    std::string tokenLTP;
    std::string ratingCode;
    std::string songLength;
    std::string popmRating;
    std::string albumID;
    static std::string s_artistInterval{"0"};
    std::string s_selectedTrackPath;
    StringVector2D artistIntervalVec = readCSV(appDataPathstr.toStdString()+"/artistsadj.txt");
    //std::cout << "Starting getline to read artistsadj.txt file into artistsadjVec." << std::endl;
    std::vector<std::string>ratedabbrvect;
    //ratedabbrvect.reserve(20000);
    if (Constants::verbose == true) std::cout << "................" << std::endl;
    // Outer loop: iterate through ratedSongsTable in the file "ratedlib.dsv"
    // Need to store col values for song path (8), LastPlayedDate (17), playlist position (will be obtained from cleanedplaylist), artist (19),
    // rating (29); artist interval will be obtained from artistsadj.txt
    while (std::getline(primarySongsTable, str1)) {  // Declare variables applicable to all rows
        std::istringstream iss(str1); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        //std::cout << "While iterating lines from cleanlib.dsv into vector ratedabbrvect, start token loop." << std::endl;
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, '^')) {
            // TOKEN PROCESSING - COL 2
            if (tokenCount == 2){albumID = token;} // store albumID variable
            // TOKEN PROCESSING - COL 8
            if (tokenCount == 8){songPath = token;} // store song path variable
            // TOKEN PROCESSING - COL 12
            if (tokenCount == 12){songLength = token;} // store song length variable
            // TOKEN PROCESSING - COL 13
            if (tokenCount == 13){popmRating = token;} // store song length variable
            // TOKEN PROCESSING - COL 17
            if ((tokenCount == 17) && (popmRating != "0") && (token != "0")){tokenLTP = token;} // store LastPlayedDate in SQL Time
            // TOKEN PROCESSING - COL 19
            if (tokenCount == 19) {selectedArtistToken = token;}// artist
            // TOKEN PROCESSING - COL 29
            if (tokenCount == 29){ratingCode = token;} // store rating variable
            // TOKEN PROCESSING - Artist Interval
            // Using 2D vector using artistIntervalVec, assign interval of artist matching
            // selectedArtistToken to s_artistInterval variable
            std::string artistsadjartGp;
            for(size_t i = 0; i < artistIntervalVec.size(); i++){
                artistsadjartGp = artistIntervalVec[i][0];
                if (artistsadjartGp == selectedArtistToken){
                    s_artistInterval = artistIntervalVec[i][4];
                }
            }
            // TOKEN PROCESSING - Playlist Position: assign playlist position of 0. See ratedabbrvect.push_back below
            ++ tokenCount;
        }
        //Send all of these to the new vector, taking the place of ratedabbr.txt
        // Write lasttimeplayed, rating code, artist, songpath, songlength, repeat interval, and playlist position of zero to
        // the output file if not currently the header row, and is a rated track
        if (ratingCode != "0"){
            //std::cout << "Processing rated string #: " <<ratedabbrvect.size() << "." << "\n";
            if ((selectedArtistToken != "Custom2") && (selectedArtistToken != "Artist")){
                ratedabbrvect.push_back(tokenLTP+","+ratingCode+","+selectedArtistToken+","+songPath+","+songLength+","+s_artistInterval+","+albumID+",0");
            }
        }
    }
    ratedabbrvect.shrink_to_fit();
    if (Constants::verbose == true) std::cout << "Do final sort of ratedabbrvect and write to file." << std::endl;
    std::sort (ratedabbrvect.begin(), ratedabbrvect.end());
    for (std::size_t i = 0 ;  i < ratedabbrvect.size(); i++){
        ratedabbr << ratedabbrvect[i] << "\n";}
    primarySongsTable.close(); // Close cleanlib and vectors
    ratedabbr.close();
    ratedabbrvect.shrink_to_fit();
    if (Constants::verbose == true) std::cout << "...finished!" << std::endl;
}
