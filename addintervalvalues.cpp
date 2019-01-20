#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdio.h>

void addIntervalValues() {
    bool customArtistID{true}; // fix when user selection function is created
    std::fstream filestrinterval;
    filestrinterval.open ("rated.dsv");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "addIntervalValues: Error opening rated.dsv file" << std::endl;}
    std::string ratedlibrary = "rated.dsv"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "addIntervalValues: Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream outfratedint("ratedlib.dsv"); // output file for writing rated.dsv with added artist intervals
    std::string str1; // store the string for artistsadj.txt
    std::string str2; // store the string for ratedlib.dsv
    std::string selectedArtistToken; // Artist variable from rated.dsv
    std::string currentArtistInterval; // token is the contents of each column of data
    std::string currentArtist; // Artist variable from artistsadj.txt
    std::string delim{"^"};

    // Outer loop: iterate through ratedSongsTable in the file "rated.dsv" then use vector of str1 to
    // populate col36 of ratedlib.dsv with the header of ArtistInterval
    while (std::getline(ratedSongsTable, str2))
    {   // Declare variables applicable to all rows
        std::istringstream iss(str2); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, '^'))
        {
            // TOKEN PROCESSING - COL 1
            if ((tokenCount == 1) && (customArtistID == false)) {selectedArtistToken = token;}// if 'non-custom' artist is selected use this code
            // TOKEN PROCESSING - COL 19
            if ((tokenCount == 19) && (customArtistID == true)) {selectedArtistToken = token;}// if custom artist grouping is selected use this code
            ++ tokenCount;
        }
        // First loop: iterate through each column (token) of current row of artistsadj.txt
        // to get the artist name and interval to write interval to ratedlib.dsv
        std::fstream artistintervals;  // Next ensure artistsadj.txt is ready to open
        artistintervals.open ("artistsadj.txt");
        if (artistintervals.is_open()) {artistintervals.close();}
        else {std::cout << "addIntervalValues: Error opening artistsadj.txt file." << std::endl;}
        std::string artistIntTable = "artistsadj.txt"; // now we can use it as input file
        std::ifstream artistadjcsv(artistIntTable); // Open artistsadj.txt as ifstream
        if (!artistadjcsv.is_open())
        {
            std::cout << "addIntervalValues: Error opening artistadjcsv." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        while (std::getline(artistadjcsv, str1))
        {
            std::istringstream iss(str1); // str is the string of each row
            int tokenArtistsCount{0}; //token count is the number of delimiter characters within str
            std::string token; // token is the contents of each column of data
            while (std::getline(iss, token, ','))
            {
                // TOKEN PROCESSING - COL 0
                if (tokenArtistsCount == 0) {currentArtist = token;}

                // TOKEN PROCESSING - COL 4
                if (tokenArtistsCount == 4) {currentArtistInterval = token;}
                //  }
                ++tokenArtistsCount;
            }
            if (selectedArtistToken == currentArtist){
                outfratedint << str2 << delim << currentArtistInterval << "\n"; // Write artist to clean file
            }
            continue; // Resume rated.dsv next row, beginning with Col 0
        }
        // Completed all rows of artistadjcsv
        artistadjcsv.close(); // Must close artistadjcsv.dsv here so it can reopen for the next row of the rated.dsv file
        // Add header row with new Col if reading the first row from rated.dsv
        if ((selectedArtistToken == "Artist")|| (selectedArtistToken == "Custom2")){
            outfratedint << str2 << delim << "ArtistInterval" << "\n";
        }
    }
    // Completed all rows of rated.dsv
    ratedSongsTable.close(); // Close rated.dsv and output file
    outfratedint.close();
}
