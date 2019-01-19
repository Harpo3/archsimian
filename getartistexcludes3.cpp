#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdio.h>

void getExtendedExcludes(long *_shistCount, int *_splaylistSize)
{
    //bool customArtistID{true}; // fix when user selection function is created
    std::fstream filestrinterval;
    filestrinterval.open ("ratedabbr.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "getExtendedExcludes: Error opening ratedabbr.txt file." << std::endl;}
    std::string ratedlibrary = "ratedabbr.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "getExtendedExcludes: Error opening ratedabbr.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream playlistPosList("histposlist.txt"); // output file for writing history not in playlist for longer artist intervals

    std::string str1; // store the string for ratedabbr.txt
    bool notInPlaylist{0}; // Artist variable from ratedabbr.txt
    std::string currentArtistInterval; // token is the contents of each column of data
//  std::string currentArtist; // Artist variable from artistsadj.txt
    std::string tokenLTP;
    std::string selectedArtistToken; // Artist variable from ratedabbr.txt
    std::string songpath;
    std::string ratingCode;
    std::string songLength;
    std::string artistInterval;

    std::vector<std::string>histvect;

    // Outer loop: iterate through ratedSongsTable in the file "ratedabbr.txt"
    while (std::getline(ratedSongsTable, str1))
    {  // Declare variables applicable to all rows
        std::istringstream iss(str1); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str

        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, ','))
        {
            // TOKEN PROCESSING - COL 0
            if ((tokenCount == 0) && (token != "0"))
            {tokenLTP = token;}// get LastPlayed Date in SQL Time
            // TOKEN PROCESSING - COL 1
            if ((tokenCount == 1) && (token == "0"))
            {notInPlaylist = true;}// store temp variable to check that item is not in playlist
            // TOKEN PROCESSING - COL 2
            if (tokenCount == 2) {ratingCode = token;}// store rating variable
           // TOKEN PROCESSING - COL 3
            if (tokenCount == 3) {selectedArtistToken = token;}// artist is selected
            // TOKEN PROCESSING - COL 5
            if (tokenCount == 5) {songLength = token;}// store song length variable
            // TOKEN PROCESSING - COL 6
            if (tokenCount == 6) {artistInterval = token;}// store artist interval
            ++ tokenCount;
        }
        if (notInPlaylist == true) {histvect.push_back(tokenLTP+","+selectedArtistToken+","+songLength+","+ratingCode+","+artistInterval);
            continue;}
    }
    std::sort (histvect.begin(), histvect.end());
    std::reverse (histvect.begin(), histvect.end());

    std::vector<std::string>::const_iterator begin = histvect.begin();
    std::vector<std::string>::const_iterator last = histvect.begin() + *_shistCount;
    std::vector<std::string> new_histvect(begin, last);

    // Add a column to histposlist.txt to list the position number beyond the playlist count
    //std::cout << "getExtendedExcludes (getartistexcludes3): Playlist size is now:" << *_splaylistSize << std::endl;

    for (std::size_t i = 0 ;  i < new_histvect.size(); i++){
        playlistPosList << new_histvect[i] << "," << i + 1 + *_splaylistSize << "\n";}

    histvect.shrink_to_fit();
    new_histvect.shrink_to_fit();
    ratedSongsTable.close();
    playlistPosList.close();
}
