#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

// Since adding tracks is iterative, we will limit writes to the ratedlib.dsv file by first doing a smaller subset
// function getSubset() to create the file ratedabbr.txt, which will then be used to do this function for track selection,
// then after all tracks have been added using this function, a separate function finalizeRatedLib()
// will be used to recreate ratedlib.dsv. Only run if needUpdate==1.
void getSubset() {
    bool customArtistID{true}; // fix when user selection function is created
    std::fstream filestrinterval;
    filestrinterval.open ("ratedlib.dsv");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "Error opening ratedlib.dsv file." << std::endl;}
    std::string ratedlibrary = "ratedlib.dsv"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()){
        std::cout << "Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for ratedlib.dsv
    std::string selectedArtistToken; // Artist variable from
    std::string songPath;
    std::string tokenLTP;
    std::string playlistPos;
    std::string ratingCode;
    std::string songLength;
    std::string artistInterval;
    std::string s_selectedTrackPath;
    std::vector<std::string>ratedabbrvect;
    std::ofstream ratedabbr("ratedabbr.txt"); // output file for subset table used for playlist selections
    // Outer loop: iterate through ratedSongsTable in the file "ratedlib.dsv"
    // Need to store col values for Artist (1 or 19), song path (8), LastPlayedDate (17), playlist position (18), rating (29),
    while (std::getline(ratedSongsTable, str1)) {  // Declare variables applicable to all rows
        std::istringstream iss(str1); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, '^')) {
            // TOKEN PROCESSING - COL 1
            if ((tokenCount == 1) && (customArtistID == false)) {selectedArtistToken = token;}// if 'non-custom' artist is selected use this code
            // TOKEN PROCESSING - COL 8
            if (tokenCount == 8){songPath = token;} // store song path variable
            // TOKEN PROCESSING - COL 12
            if (tokenCount == 12){songLength = token;} // store song length variable
            // TOKEN PROCESSING - COL 17
            if ((tokenCount == 17) && (token != "0")){tokenLTP = token;} // store LastPlayedDate in SQL Time
            // TOKEN PROCESSING - COL 18
            if (tokenCount == 18){playlistPos = token;} // store playlist position
            // TOKEN PROCESSING - COL 19
            if ((tokenCount == 19) && (customArtistID == true)) {selectedArtistToken = token;}//if custom artist is selected use this code
            // TOKEN PROCESSING - COL 29
            if (tokenCount == 29){ratingCode = token;} // store rating variable
            // TOKEN PROCESSING - COL 36
            if (tokenCount == 36){artistInterval = token;} // store artistInterval variable
            ++ tokenCount;
        }
        //Send all of these to the new vector
        // Write artist, count, adjusted count, artist factor, and repeat interval to the output file if not the header row
        if ((selectedArtistToken != "Custom2") && (selectedArtistToken != "Artist")){
            ratedabbrvect.push_back(tokenLTP+","+playlistPos+","+ratingCode+","+selectedArtistToken+","+songPath+","+songLength+","+artistInterval);}
    }
    std::sort (ratedabbrvect.begin(), ratedabbrvect.end());
    for (std::size_t i = 0 ;  i < ratedabbrvect.size(); i++){
        ratedabbr << ratedabbrvect[i] << "\n";}
    ratedSongsTable.close();
    ratedabbr.close();
    ratedabbrvect.shrink_to_fit();
}
