#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QString>
#include <sstream>
#include <fstream>
#include <iostream>
#include "utilities.h"
#include "constants.h"


// Function to get track count per artist

void getArtistTrackCount(){
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::ifstream cleanlib;  // First ensure cleanlib.dsv is ready to open
    cleanlib.open (appDataPathstr.toStdString()+"/cleanlib.dsv");
    if (cleanlib.is_open()) {cleanlib.close();}
    else {
        std::cout << "getArtistTrackCount: Error opening cleanlib.dsv file." << std::endl;
        Logger ("getArtistTrackCount: Error opening cleanlib.dsv file.");
    }
    std::string cleanlibSongsTable = appDataPathstr.toStdString()+"/cleanlib.dsv";    // Now we can use it as input file
    std::ifstream SongsTable(cleanlibSongsTable);    // Open cleanlib.dsv as ifstream
    if (!SongsTable.is_open())
    {
        std::cout << "getArtistTrackCount: Error opening SongsTable." << std::endl;
        Logger ("getArtistTrackCount: Error opening SongsTable.");
        std::exit(EXIT_FAILURE); // Otherwise, quit
    }
    std::string str;
    std::vector<std::string> artistrawcountVec; // vector list of strings (artistrawcountVec) to output a count of duplicate entries
    //std::vector<std::string> artistsongcountVec; // vector to store list of counts
    std::ofstream artistList(appDataPathstr.toStdString()+"/artists.txt"); // output file for writing artists list without dups using vector
    //  Outer loop: iterate through rows of SongsTable
    while (std::getline(SongsTable, str))
    {   // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        bool ratingCode{false};
        std::string selectedArtistToken; //used to filter rows where track has a playlist position;
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row and push all rated artist entries into a vector of strings
        while (std::getline(iss, token, '^'))
        {
            if (tokenCount == Constants::kColumn19)  {selectedArtistToken = token;}//  artist grouping
            if ((tokenCount == Constants::kColumn29) && (token != "0")) {ratingCode = true;}// Determine if a rated track and set bool
            ++ tokenCount;
        }
        if (ratingCode) {artistrawcountVec.push_back(selectedArtistToken);}
    }
    // Create a map to store the frequency of each element in vector
    std::map<std::string, int> countMap;
    // Iterate over the vector and store the frequency of each element in map
    for (auto & elem : artistrawcountVec)
    {
        auto result = countMap.insert(std::pair<std::string, int>(elem, 1));
        if (result.second == false)
            result.first->second++;
    }
    // Iterate over the map and print items whose value is greater than 0
    for (auto & elem : countMap)
    {
        // If frequency count is greater than 0 then consider it (even if count only 1) a duplicate element
        if (elem.second > 0)
        {
            artistList << elem.first << "," << elem.second << '\n';
        }
    }
    artistrawcountVec.shrink_to_fit();
    artistList.close();
}

// Function to read cleanlib.dsv
/*

void getArtistAdjustedCount2(const double *_syrsTillRepeatCode3factor,const double *_syrsTillRepeatCode4factor,const double *_syrsTillRepeatCode5factor,
                            const double *_syrsTillRepeatCode6factor,const double *_syrsTillRepeatCode7factor,const double *_syrsTillRepeatCode8factor,
                            const int *_srCode3TotTrackQty,const int *_srCode4TotTrackQty,const int *_srCode5TotTrackQty,
                             const int *_srCode6TotTrackQty,const int *_srCode7TotTrackQty,const int *_srCode8TotTrackQty)
{
    getArtistTrackCount(); // Get artists.txt file

    // Open cleanlib.dsv into a vector cleanlibVec

    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::ifstream cleanlib2;  // Ensure cleanlib.dsv is ready to open
    cleanlib2.open (appDataPathstr.toStdString()+"/cleanlib.dsv");
    if (cleanlib2.is_open()) {cleanlib2.close();}
    else {std::cout << "getArtistAdjustedCount: Error cleanlib2 opening cleanlib.dsv file." << std::endl;}
    std::string cleanlibSongsTable2 = appDataPathstr.toStdString()+"/cleanlib.dsv"; // now we can use it as input file
    StringVector2D cleanlibVec = readDSV(appDataPathstr.toStdString()+"/cleanlib.dsv");
    std::string selectedArtist; //used to store artist name read from token;
    std::string selectedRating; //used to store rating for current row;
    for(auto & i : cleanlibVec){ // Read each row element from cleanlibVec
        selectedArtist = i[Constants::kColumn19];
        selectedRating = i[Constants::kColumn29];
    }

    // Open artists.txt into vector artistsVec

    StringVector2D artistsVec = readCSV(appDataPathstr.toStdString()+"/artists.txt");
    std::string currentArtist; //used to store artist name read from token;
    std::string currentArtistTrackCount; //used to store artist track count for current row;
    for(auto & j : artistsVec){ // Read each row element from artistsVec
        currentArtist = j[Constants::kColumn0];
        currentArtistTrackCount = j[Constants::kColumn1];

        // For the currently selected currentArtist and associated currentArtistTrackCount find matching artist
        // in cleanlibVec

        for (auto & i : cleanlibVec) {
            selectedArtist == i[Constants::kColumn19])
            selectedRating == i[Constants::kColumn29]
            if (currentArtist == selectedArtist) {
                    //set temp variable to check when the rating token is checked next
                    -- countdown;


                if (artistMatch == true) {
                    //selectedRating = token;
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
            }
            // Increment to the next column of row in cleanlib.dsv
        }
    }


    // All entries in the artists.txt file completed and adjusted values written to new file. Close files opened for reading and writing
    cleanlibVec.shrink_to_fit();
    artistsVec.shrink_to_fit();
    removeAppData("artists.txt");
    removeAppData("artists2.txt");



}
*/

// Function to claculate an adjusted track count for each artist using a weighting based on assigned rating for each track
void getArtistAdjustedCount(const double *_syrsTillRepeatCode3factor,const double *_syrsTillRepeatCode4factor,const double *_syrsTillRepeatCode5factor,
                            const double *_syrsTillRepeatCode6factor,const double *_syrsTillRepeatCode7factor,const double *_syrsTillRepeatCode8factor,
                            const int *_srCode3TotTrackQty,const int *_srCode4TotTrackQty,const int *_srCode5TotTrackQty,
                            const int *_srCode6TotTrackQty,const int *_srCode7TotTrackQty,const int *_srCode8TotTrackQty)
{
    getArtistTrackCount();
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::ifstream cleanlib2;  // Ensure cleanlib.dsv is ready to open
    cleanlib2.open (appDataPathstr.toStdString()+"/cleanlib.dsv");
    if (cleanlib2.is_open()) {cleanlib2.close();}
    else {
        std::cout << "getArtistAdjustedCount: Error cleanlib2 opening cleanlib.dsv file." << std::endl;
        Logger ("getArtistAdjustedCount: Error cleanlib2 opening cleanlib.dsv file.");
    }
    std::string cleanlibSongsTable2 = appDataPathstr.toStdString()+"/cleanlib.dsv"; // now we can use it as input file
    std::ifstream artists2;  // Next ensure artists.txt is ready to open
    artists2.open (appDataPathstr.toStdString()+"/artists.txt");
    if (artists2.is_open()) {artists2.close();}
    else {
        std::cout << "getArtistAdjustedCount: Error artists2 opening artists.txt file." << std::endl;
        Logger ("getArtistAdjustedCount: Error artists2 opening artists.txt file.");
    }
    std::string artistsTable2 = appDataPathstr.toStdString()+"/artists.txt"; // now we can use it as input file
    std::ifstream artistcsv(artistsTable2); // Open artists.txt as ifstream
    if (!artistcsv.is_open())
    {
        std::cout << "getArtistAdjustedCount: Error opening artists.txt." << std::endl;
        Logger ("getArtistAdjustedCount: Error opening artists.txt file.");
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for artists.txt
    std::string str2; // store the string for cleanlib.dsv
    std::ofstream outartists2(appDataPathstr.toStdString()+"/artistsadj.txt"); // Create ostream file to collect artists and adjusted counts
    std::string currentArtist;
    int currentArtistCount{0};
    // Outer loop: iterate through artist, track count in the file "artists.txt"
    // For each artist i with a track count more than 1, store a temp variable vSelArtist for iterating
    // If artist count is 1, set adjusted count to 1 and store adjCount of 1 in Custom 3
    while (std::getline(artistcsv, str1))
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
            if (tokenArtistsCount == Constants::kColumn0) {currentArtist = tokenArtist;}
            //std::cout << "Current artist from artists.txt is " << currentArtist << std::endl;
            if (tokenArtistsCount == Constants::kColumn1) {currentArtistCount = std::stoi(tokenArtist);}
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
            Logger ("getArtistAdjustedCount: Error opening SongsTable2.");
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
                if (tokenCount == Constants::kColumn19)  {
                    selectedArtistToken = token;
                    if (currentArtist == selectedArtistToken) {
                        //set temp variable to check when the rating token is checked next
                        artistMatch = true;
                        -- countdown;
                    }
                }
                if (tokenCount == Constants::kColumn29) {
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
            //continue; // Resume cleanlib.dsv next row, beginning with Col 0
        }
        // Completed all rows of cleanlib.dsv

        double s_totalAdjRatedQty = (*_syrsTillRepeatCode3factor * *_srCode3TotTrackQty)+(*_syrsTillRepeatCode4factor * *_srCode4TotTrackQty)
                + (*_syrsTillRepeatCode5factor * *_srCode5TotTrackQty) +(*_syrsTillRepeatCode6factor * *_srCode6TotTrackQty)
                +(*_syrsTillRepeatCode7factor * *_srCode7TotTrackQty) + (*_syrsTillRepeatCode8factor * *_srCode8TotTrackQty);

        // NEW : this is what is slowing down the function. Should read clean.dsv into a vector instead.

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
    //artists.shrink_to_fit();
    removeAppData("artists.txt");
    removeAppData("artists2.txt");
}
