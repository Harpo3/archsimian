#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <iomanip>
#include <time.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <map>
#include <chrono>
#include <random> // for std::mt19937
#include <ctime> // for std::time
#include "userconfig.h"
#include "constants.h"
#include "lastplayeddays.h"


inline bool doesFileExist (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

typedef std::vector<std::string> StringVector;
typedef std::vector<StringVector> StringVector2D;

using StringVector = std::vector<std::string>;
using StringVector2D = std::vector<StringVector>;

StringVector2D readCSV(std::string filename)
{
    char separator = ',';
    StringVector2D result;
    std::string row, item;

    std::ifstream in(filename);
    while(getline(in,row))
    {
        StringVector R;
        std::stringstream ss(row);
        while (getline(ss,item,separator))R.push_back(item);
        result.push_back(R);
    }
    in.close();
    return result;
}

bool recentlyUpdated(std::string *_smmBackupDBDir)
{
    bool existResult{0};
    bool refreshNeededResult{0};
    existResult = doesFileExist(Constants::existLibName);// See inline function at top
    if (Constants::verbose == true) std::cout << "recentlyUpdated(): doesFileExist() result for cleanlib.dsv is " << existResult << std::endl;
    if (existResult == 0) {refreshNeededResult = 1;}
    // If the lib file exists, Get the epoch date for the MM.DB file
    // and see which file is older
    if (existResult == 1){
        std::string mmdbdir = *_smmBackupDBDir; // z: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath);
        std::string mmpath = mmdbdir + "/MM.DB";
        struct stat stbuf1;
        stat(mmpath.c_str(), &stbuf1);
        localtime(&stbuf1.st_mtime); // or gmtime() depending on what you want
        //printf("Modification time for MM.DB is %ld\n",stbuf1.st_mtime);
        if (Constants::verbose == true) std::cout << "MM.DB is " << stbuf1.st_mtime << std::endl;
        // Now get the date for the cleanlib.csv file
        struct stat stbuf2;
        stat(Constants::existLibName, &stbuf2);
        localtime(&stbuf2.st_mtime);
        //printf("Modification time for cleanlib.csv is %ld\n",stbuf2.st_mtime);
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



// Above functions and constants can be removed after referencing include for lastplayeddays.h and constants.h

// Function to remove all spaces from a given string
std::string removeSpaces(std::string str)
{
    str.erase(remove(str.begin(), str.end(), ' '), str.end());
    return str;
}

// Function to return a vector as a single DSV string with carat delimiter
std::string getChgdDSVStr(std::vector<std::string> const &input,std::string chgdString) {
    std::ostringstream oschgdString;
    for (unsigned long i = 0; i < input.size(); i++) {
        oschgdString << input.at(i) << '^';
    }
    chgdString = oschgdString.str();
    return chgdString;
}

// Function to return all vector contents to a single dir path string using a '/' delimiter
std::string getChgdDirStr(std::vector<std::string> const &input,std::string chgdString) {
    std::ostringstream oschgdString;
    std::string s_musiclibrarydirname = "/media/sdc2"; // fix in main function**********************************
    for (unsigned long i = 1; i < input.size(); i++) {
        oschgdString << '/' << input.at(i);
    }
    chgdString = s_musiclibrarydirname + oschgdString.str();
    return chgdString;
}

void getLibrary()
{
    std::ifstream filestr1;
    filestr1.open ("libtable.dsv");
    if (filestr1.is_open()) {filestr1.close();}
    else {std::cout << "getLibrary: Error opening libtable.dsv file." << std::endl;}
    std::string databaseFile = "libtable.dsv"; // now we can use it as input file
    std::ofstream outf("cleanlib.dsv"); // output file for writing clean track paths
    std::ifstream primarySongsTable(databaseFile);
    std::string s_musiclibrarydirname = "/media/sdc2"; // fix in main function**********************************

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

        // Within tokens vector, open dirPathTokens vector tp tokenize dir path of tokens[8] using '\' as delimiter
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
        dirPathTokens.at(0) = s_musiclibrarydirname;
        songPath1 = getChgdDirStr(dirPathTokens,songPath1);
        tokens.at(8) = songPath1;
        dirPathTokens.shrink_to_fit();

         //Printing the dirPathTokens vector
        //for(unsigned long i = 0; i < dirPathTokens.size(); i++)
        //    std::cout << dirPathTokens[i] << '\n';

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

void getDBStats(int *_srCode0TotTrackQty,int *_srCode0MsTotTime,int *_srCode1TotTrackQty,int *_srCode1MsTotTime,
                 int *_srCode3TotTrackQty,int *_srCode3MsTotTime,int *_srCode4TotTrackQty,int *_srCode4MsTotTime,
                 int *_srCode5TotTrackQty,int *_srCode5MsTotTime,int *_srCode6TotTrackQty,int *_srCode6MsTotTime,
                 int *_srCode7TotTrackQty,int *_srCode7MsTotTime,int *_srCode8TotTrackQty,int *_srCode8MsTotTime,
                 int *_sSQL10TotTimeListened,int *_sSQL10DayTracksTot,double *_sSQL20TotTimeListened,
                 int *_sSQL20DayTracksTot,double *_sSQL30TotTimeListened,int *_sSQL30DayTracksTot,double *_sSQL40TotTimeListened,
                 int *_sSQL40DayTracksTot,double *_sSQL50TotTimeListened,int *_sSQL50DayTracksTot,double *_sSQL60TotTimeListened,
                 int *_sSQL60DayTracksTot) {
    std::ifstream filestr1;
    filestr1.open ("cleanlib.dsv");
    if (filestr1.is_open()) {filestr1.close();}
    else {std::cout << "getDBStats: Error opening cleanlib.dsv file." << std::endl;}
    std::string databaseFile = "cleanlib.dsv"; // now we can use it as input file
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
                tempLastPlayedDate = std::atof(token.c_str()); //valid C++ syntax to convert a token string to a double.
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

void getArtistAdjustedCount(double *_syrsTillRepeatCode3factor,double *_syrsTillRepeatCode4factor,double *_syrsTillRepeatCode5factor,
                            double *_syrsTillRepeatCode6factor,double *_syrsTillRepeatCode7factor,double *_syrsTillRepeatCode8factor,
                            int *_srCode3TotTrackQty,int *_srCode4TotTrackQty,int *_srCode5TotTrackQty,
                            int *_srCode6TotTrackQty,int *_srCode7TotTrackQty,int *_srCode8TotTrackQty)
{
    //std::cout << "Working on artist counts and factors. This will take a few seconds...";
    std::ifstream cleanlib;  // First ensure cleanlib.dsv is ready to open
    cleanlib.open ("cleanlib.dsv");
    if (cleanlib.is_open()) {cleanlib.close();}
    else {std::cout << "getArtistAdjustedCount: Error opening cleanlib.dsv file." << std::endl;}
    std::string cleanlibSongsTable = "cleanlib.dsv";    // Now we can use it as input file
    std::ifstream SongsTable(cleanlibSongsTable);    // Open cleanlib.dsv as ifstream
    if (!SongsTable.is_open())
    {
        std::cout << "getArtistAdjustedCount: Error opening SongsTable." << std::endl;
        std::exit(EXIT_FAILURE); // Otherwise, quit
    }
    std::string str; // Create ostream file to log artists and duplicates; dups will be used to create a vector with # tracks per artist
    std::ofstream outartists("artists2.txt"); // output file for writing artists list with dups
    //  Outer loop: iterate through rows of SongsTable
    while (std::getline(SongsTable, str))
    {   // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        bool ratingCode{false};
        std::string selectedArtistToken; //used to filter rows where track has a playlist position;
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, '^'))
        {
            // TOKEN PROCESSING - COL 19
            if (tokenCount == 19)  {selectedArtistToken = token;}//  artist grouping
            // TOKEN PROCESSING - COL 29
            if ((tokenCount == 29) && (token != "0")) {ratingCode = true;}// if custom artist grouping is selected use this code
            ++ tokenCount;
        }
        if (ratingCode == true) {outartists << selectedArtistToken << "\n"; }// Write artist to clean file if rated
    }
    SongsTable.close();    // Close files opened for reading and writing. SongsTable will be reopened shortly as cleanlibSongsTable2
    outartists.close();
    std::map<std::string, int> countMap; // Create a map for two types, string and int
    std::vector<std::string> artists; // Now create vector
    std::ifstream myfile("artists2.txt"); // Input file for vector
    std::ofstream artistList("artists.txt"); // output file for writing artists list without dups using vector
    std::string line;
    while ( std::getline(myfile, line) ) { //std::cout << "Vector Size is now " << artists.size() << " lines." << std::endl;
        if ( !line.empty() )
            artists.push_back(line);
    }
    for (auto & elem : artists)  // Iterate over the vector and store the frequency of each element in map
    {
        auto result = countMap.insert(std::pair<std::string, int>(elem, 1));
        if (result.second == false)
            result.first->second++;
    }
    // Iterate over the map
    for (auto & elem : countMap)
    {
        // If frequency count is greater than 0 then its element count is captured
        if (elem.second > 0)
        {
            artistList << elem.first << "," << elem.second << "\n";
        }
    }
    myfile.close();
    artistList.close();
    if( remove( "artists2.txt" ) != 0 )
        perror( "getArtistAdjustedCount: Error deleting file" );
    std::ifstream cleanlib2;  // Ensure cleanlib.dsv is ready to open
    cleanlib2.open ("cleanlib.dsv");
    if (cleanlib2.is_open()) {cleanlib2.close();}
    else {std::cout << "getArtistAdjustedCount: Error cleanlib2 opening cleanlib.dsv file." << std::endl;}
    std::string cleanlibSongsTable2 = "cleanlib.dsv"; // now we can use it as input file
    std::ifstream artists2;  // Next ensure artists.txt is ready to open
    artists2.open ("artists.txt");
    if (artists2.is_open()) {artists2.close();}
    else {std::cout << "getArtistAdjustedCount: Error artists2 opening artists.txt file ." << std::endl;}
    std::string artistsTable2 = "artists.txt"; // now we can use it as input file
    std::ifstream artistcsv(artistsTable2); // Open artists.txt as ifstream
    if (!artistcsv.is_open())
    {
        std::cout << "Error opening artistcsv." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for artists.txt
    std::string str2; // store the string for cleanlib.dsv
    std::ofstream outartists2("artistsadj.txt"); // Create ostream file to collect artists and adjusted counts
    std::string currentArtist;
    int currentArtistCount{0};
    // Outer loop: iterate through artist, track count in the file "artists.txt"
    // For each artist i with a track count more than 1, store a temp variable vSelArtist for iterating
    // If artist count is 1, set adjusted count to 1 and store adjCount of 1 in Custom 3
    while ( std::getline(artistcsv, str1) )
    {
        double interimAdjCount{0.0};
        int tokenArtistsCount{0};
        std::istringstream issArtists(str1); // str is the string of each row
        std::string tokenArtist; // token is the content of each column of data in artists.txt
        int countdown{0};
        // First loop: iterate through each column (token) of current row of artists.txt
        // to get the artist name and number of tracks to find in cleanlib.dsv
        while (std::getline(issArtists, tokenArtist, ','))
        {
            countdown = currentArtistCount + 1;
            // TOKEN PROCESSING - COL 0
            if (tokenArtistsCount == 0) {currentArtist = tokenArtist;}
            //std::cout << "Current artist from artists.txt is " << currentArtist << std::endl;
            // TOKEN PROCESSING - COL 1
            if (tokenArtistsCount == 1) {currentArtistCount = std::stoi(tokenArtist);}
            ++tokenArtistsCount;
        }
        countdown = currentArtistCount;
        // Second loop uses cleanlib.dsv
        // open cleanlib.dsv and vector for artist track count and calculate adjusted track count, and write to Custom3
        // Open cleanlib.dsv as ifstream
        std::ifstream SongsTable2(cleanlibSongsTable2);
        if (!SongsTable2.is_open())
        {
            std::cout << "getArtistAdjustedCount: Error opening SongsTable2." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        while (std::getline(SongsTable2, str2) && countdown != 0) //Check every row until all artist's tracks found
        {   // Declare variables applicable to all rows
            std::istringstream iss(str2); // str is the string of each row
            std::string token; // token is the contents of each column of data
            std::string selectedArtistToken; //used to store artist name read from token;
            std::string selectedRating; //used to store rating for current row;
            int tokenCount{0}; //token count is the number of delimiter characters within str
            bool artistMatch = false;
            // Inner loop within second loop: iterate through each column (token) of row to find and match artist
            while (std::getline(iss, token, '^'))
            {
                // TOKEN PROCESSING - COL 19  artist token
                if (tokenCount == 19)  {
                    selectedArtistToken = token;
                    if (currentArtist == selectedArtistToken) {
                        //set temp variable to check when the rating token is checked next
                        artistMatch = true;
                        -- countdown;
                        //                       std::cout << " countdown is now " << std::to_string(countdown) << std::endl;
                    }
                }
                // TOKEN PROCESSING - COL 29 get rating and store for current row
                if (tokenCount == 29) {
                    if (artistMatch == 1) {
                        selectedRating = token;
                        // Now evaluate the rating using factors and calculate adjusted track value
                        // Increment adjusted values as each track is found using the rating factor stats collected
                        if (selectedRating == "1") {interimAdjCount = interimAdjCount + 1;}
                        if (selectedRating == "3") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode3factor;}
                        if (selectedRating == "4") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode4factor;}
                        if (selectedRating == "5") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode5factor;}
                        if (selectedRating == "6") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode6factor;}
                        if (selectedRating == "7") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode7factor;}
                        if (selectedRating == "8") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode8factor;}
                    }
                }
                // Increment to the next column of row in cleanlib.dsv
                ++ tokenCount;
            }
            continue; // Resume cleanlib.dsv next row, beginning with Col 0
        }
        // Completed all rows of cleanlib.dsv
        double s_totalAdjRatedQty = (*_syrsTillRepeatCode3factor * *_srCode3TotTrackQty)+(*_syrsTillRepeatCode4factor * *_srCode4TotTrackQty)
                + (*_syrsTillRepeatCode5factor * *_srCode5TotTrackQty) +(*_syrsTillRepeatCode6factor * *_srCode6TotTrackQty)
                +(*_syrsTillRepeatCode7factor * *_srCode7TotTrackQty) + (*_syrsTillRepeatCode8factor * *_srCode8TotTrackQty);
        SongsTable2.close(); // Must close cleanlib.dsv here so it can reopen for the next artist on the artists.txt file
        if (interimAdjCount < currentArtistCount) {interimAdjCount = currentArtistCount;} // Adjusted count must be at least one if there is one track or more
        double currentArtistFactor = (interimAdjCount / s_totalAdjRatedQty); //percentage of total adjusted tracks
        int availInterval = int(1 / currentArtistFactor);
        // Write artist, count, adjusted count, artist factor, and repeat interval to the output file if not the header row
        if (currentArtist != "Custom2") {
            outartists2 << currentArtist << "," << int(currentArtistCount) << "," << int(interimAdjCount) <<
                           "," << currentArtistFactor << "," << availInterval << std::endl;
        }
        // Resume with next artist on the artists.txt file
    }
    // All entries in the artists.txt file completed and adjusted values written to new file. Close files opened for reading and writing
    artistcsv.close();
    outartists2.close();
    artists.shrink_to_fit();
    if( remove( "artists.txt" ) != 0 )
        perror( "getArtistAdjustedCount: Error deleting artists.txt file" );
}
void buildDB()
{
    if (Constants::verbose == true) std::cout << "Building the Archsimian database with artist intervals calculated....";

    const std::string cleanLibFile("cleanlib.dsv"); // remove when adding to main*****************

    //****************
    // function createDatabase - writing ratedabbr.txt using multiple vectors
    //****************
    // Since adding tracks is iterative, we will limit vector size of songs table by first doing a smaller subset
    // function getSubset() to create the file ratedabbr.txt, which will then be used for track selection functions
    std::ofstream ratedabbr("ratedabbr.txt"); // output file for subset table
    std::fstream filestrinterval;
    filestrinterval.open (cleanLibFile);
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "Error opening cleanLibFile." << std::endl;}
    std::string ratedlibrary = cleanLibFile; // now we can use it as input file
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
    static std::string s_artistInterval{"0"};
    std::string s_selectedTrackPath;

    StringVector2D artistIntervalVec = readCSV("artistsadj.txt");
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
                ratedabbrvect.push_back(tokenLTP+","+ratingCode+","+selectedArtistToken+","+songPath+","+songLength+","+s_artistInterval+",0");
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