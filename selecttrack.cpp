#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdio.h>

std::vector<std::string> split(std::string strToSplit, char delimeter){    
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter)) {
        splittedStrings.push_back(item);
    }
    return splittedStrings;
}
// Since adding tracks is iterative, limit writes to the ratedlib.dsv file by first doing a smaller subset
// function getSubset() to create the file ratedabbr.txt, which will then be used to do this function for track selection,
// then afer all tracks have been added with this function, a separate function finalizeRatedLib()
// will recreate ratedlib.dsv.
//
//Screen out tracks in ratedlib.dsv that do not match the rating of the s_ratingNextTrack variable
// Screen out tracks that have an artist that matches any when iterating through the finalartistexcludes.txt file
// Send remaining to a new vector
// Sort vector to select the oldest dated track for addition to the playlist
// Write/append the cleanedplaylist.txt file the oldest dated track found.
// Increment the playlist position count of all tracks which are greater than zero by one,
// Add the position of 1 to the newly added track
// (Excluded artists will be recalculated via a separate function)

//std::ofstream outfratedint2("extendexcludes.txt"); // output file for writing ratedabbr.txt with added artist intervals

void selectTrack(int *_sratingNextTrack){
    //std::cout << "Starting selectTrack function. Rating for next track is " << *_sratingNextTrack << std::endl;
    std::fstream filestrinterval;
    filestrinterval.open ("ratedabbr.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "Error opening ratedabbr.txt file." << std::endl;}
    std::string ratedlibrary = "ratedabbr.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()) {
        std::cout << "Error opening ratedabbr.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for ratedabbr.txt
    std::string str2; // store the string for finalartistexcludes.txt
    std::string str3; // store the string for ratedabbr.txt (second opening)
    bool notInPlaylist{0};
    std::string currentArtistInterval; // token is the contents of each column of data
    std::string currentArtist; // Artist variable from
    std::string tokenLTP;
    std::string selectedArtistToken; // Artist variable from
    std::string ratingCode;
    std::string songPath;
    std::string artistInterval;
    std::string playlistPos;
    std::string songLengtha; //just added
    std::string artistIntervala;
    std::string selectedTrackPath;
    static bool s_excludeMatch{false};
    std::vector<std::string>finaltracksvect; // New vector to store final selections
    //std::cout << "selectTrack function: Created new vector to store final selections" << std::endl;
    // Outer loop: iterate through ratedSongsTable in the file "ratedabbr.txt"
    // Need to store col values for Artist (1 or 19), song path (8), LastPlayedDate (17), playlist position (18), rating (29),
    while (std::getline(ratedSongsTable, str1)) {  // Declare variables applicable to all rows
        std::istringstream iss(str1); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, ',')) {
            // TOKEN PROCESSING - COL 0
            if ((tokenCount == 0) && (token != "0")) {tokenLTP = token;}// get LastPlayedDate in SQL Time
            // TOKEN PROCESSING - COL 1
            if (tokenCount == 1)  {playlistPos = token;}
            // TOKEN PROCESSING - COL 2
            if (tokenCount == 2) {ratingCode = token;}// store rating variable
            // TOKEN PROCESSING - COL 3
            if (tokenCount == 3) {selectedArtistToken = token;} //Selected artist token
            // TOKEN PROCESSING - COL 4
            if (tokenCount == 4) {songPath = token;}// store song path variable
            // TOKEN PROCESSING - COL 5
            if (tokenCount == 5) {songLengtha = token;} //just added
            // TOKEN PROCESSING - COL 6
            if (tokenCount == 5) {artistIntervala = token;} //just added
            ++ tokenCount;
        }
        if (playlistPos == "0") {notInPlaylist = 1;}
        else if (playlistPos != "0"){notInPlaylist = 0;}// store temp variable to check that item is not in playlist
        //std::cout << "Artist: " << selectedArtistToken;
        // For either of these variable results, continue to next row: (1) rating is not equal to s_ratingNextTrack (2) item is already on the
        // playlist, notInPlaylist == false
        if (notInPlaylist == 0) {continue;}
        //std::cout << "notInPlaylist = false:" << notInPlaylist << ". Going to next line..." << std::endl;
        else if (std::stoi(ratingCode) != *_sratingNextTrack) {continue;}
        //std::cout << ". RatingCode is: " << ratingCode << " and *_sratingNextTrack is " << *_sratingNextTrack << ". Going to next line..." << std::endl;
        else if ((std::stoi(ratingCode) == *_sratingNextTrack) && (notInPlaylist == 1)){
            //std::cout << "Track found with current rating (not yet excluded): " << selectedArtistToken << ", " << songPath << std::endl;
        }
        // If not yet skipped, open an inner loop and iterate through finalartistexcludes.txt and compare each entry against the artist token.
        // Continue to next row if a match found.
        //std::cout << "selectTrack function: ifstream finalartistexcludes." << std::endl;
        std::ifstream finalartistexcludes;  // Next ensure finalartistexcludes.txt is ready to open
        finalartistexcludes.open ("finalartistexcludes.txt");
        if (finalartistexcludes.is_open()) {finalartistexcludes.close();}
        else {std::cout << "Error opening finalartistexcludes.txt file." << std::endl;}
        std::string artistexcludes = "finalartistexcludes.txt"; // now we can use it as input file
        std::ifstream artexcludes(artistexcludes); // Open finalartistexcludes.txt as ifstream
        if (!artexcludes.is_open()) {
            std::cout << "Error opening finalartistexcludes.txt." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        s_excludeMatch = false;
        while (std::getline(artexcludes, str2)) {
            //std::cout << "Artist from finalartistexcludes.txt: "<< str2<<". Artist with sel rating from ratedabbr.txt is: "<<selectedArtistToken << std::endl;
            std::istringstream iss(str2); // str2 is the string of each row
            if (std::string(str2) == selectedArtistToken) {
                s_excludeMatch = true; // If excluded artist found, set bool to true
                //std::cout << "Excluding since selectedArtistToken is " << selectedArtistToken << " and str2 is : " << str2 << std::endl;
            }
        }

        if (s_excludeMatch == false){ // if an excluded artist is not found write to final selection vector
            //std::cout << "Track found with current rating and not on exclude list " << selectedArtistToken << " , " << songPath << std::endl;
            finaltracksvect.push_back(tokenLTP+","+songPath);}
        artexcludes.close();
        continue;
    }
    std::sort (finaltracksvect.begin(), finaltracksvect.end());
    std::string fullstring = finaltracksvect.front();
    std::vector<std::string> splittedStrings = split(fullstring, ',');
    selectedTrackPath = splittedStrings[1];
    std::cout << selectedTrackPath << std::endl;
    //std::cout << "selectTrack function: Write/append s_selectedTrackPath to the cleanedplaylist.txt file." << std::endl;
    //Write/append s_selectedTrackPath to the cleanedplaylist.txt file.
    std::ofstream playlist("cleanedplaylist.txt",std::ios::app);
    playlist << selectedTrackPath << "\n";
    playlist.close();
    ratedSongsTable.close();
    // Increment the playlist position count of all tracks which are greater than zero by one
    // and output the result to replace existing playlistposlist.txt
    std::fstream filestrinterval2;
    filestrinterval2.open ("ratedabbr.txt");
    if (filestrinterval2.is_open()) {filestrinterval2.close();}
    else {std::cout << "Error opening ratedabbr.txt file." << std::endl;}
    std::string ratedlibrary2 = "ratedabbr.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable2(ratedlibrary2);
    if (!ratedSongsTable2.is_open()) {
        std::cout << "Error opening ratedabbr.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    finaltracksvect.shrink_to_fit(); //just added
    std::vector<std::string>ratedabbrvec;
    std::string selectedArtistToken2;
    std::string newTrackPosItem;
    while (std::getline(ratedSongsTable2, str3)) {  // Declare variables applicable to all rows
        std::istringstream iss(str3); // str is the string of each row
        std::string token2; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        std::string tokenLTP2;
        std::string ratingCode2;
        std::string songLength2;
        std::string path2;
        std::string artistInterval2;
        int newPlaylistPos{0};
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token2, ',')) {
            // TOKEN PROCESSING - COL 0
            if (tokenCount == 0) {tokenLTP2 = token2;}
            // TOKEN PROCESSING - COL 1
            if ((tokenCount == 1) && (token2 != "0")) {
                //std::cout << "Was " << token2;
                newPlaylistPos = ::atoi(token2.c_str()) + 1;
                //token = std::to_string(newPlaylistPos);
                //std::cout << ", now " << token2 << "\n";
                //includeStr = 1;
            }
            // TOKEN PROCESSING - COL 2
            if (tokenCount == 2){ratingCode2 = token2;}// store rating variable
            // TOKEN PROCESSING - COL 3
            if (tokenCount == 3) {selectedArtistToken2 = token2;}  //Selected artist token
            // TOKEN PROCESSING - COL 4
            if ((tokenCount == 4) && (token2 == selectedTrackPath)) {
                newPlaylistPos = 1;
                //newExcludeArtist = selectedArtistToken2;
            }
            if (tokenCount == 4) {path2 = token2;}//Identify track selection
            // TOKEN PROCESSING - COL 5
            if (tokenCount == 5) {songLength2 = token2;}
            // TOKEN PROCESSING - COL 6
            if (tokenCount == 6) {artistInterval2 = token2;}
            ++ tokenCount;
        }
        ratedabbrvec.push_back(tokenLTP2+","+std::to_string(newPlaylistPos)+","+ratingCode2+
                               ","+selectedArtistToken2+","+path2+","+songLength2+","+artistInterval2);
    }
    ratedSongsTable2.close();
    std::ofstream playlistposlist("ratedabbr.txt");
    for(unsigned long i = 0; i < ratedabbrvec.size() ; i++) {
        playlistposlist << ratedabbrvec.at(i) << "\n";
    }
    //includeStr = 0; // The string is valid, write to clean file
    playlistposlist.close();
    ratedabbrvec.shrink_to_fit();
}
