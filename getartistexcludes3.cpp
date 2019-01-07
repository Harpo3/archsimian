#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdio.h>

void getExtendedExcludes(long *_shistCount, unsigned long *_splaylistSize)
{
    bool customArtistID{true};
    //std::ofstream outfratedint2("extendexcludes.txt"); // output file for writing rated.dsv with added artist intervals
    //int histCount{770}; // points to variable in main
    //int playlistCount{255}; // points to variable in main

    std::fstream filestrinterval;
    filestrinterval.open ("ratedlib.dsv");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "Error opening ratedlib.dsv file." << std::endl;}
    std::string ratedlibrary = "ratedlib.dsv"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream playlistPosList("histposlist.txt"); // output file for writing history not in playlist for longer artist intervals

    std::string str1; // store the string for ratedlib.dsv
    bool notInPlaylist{0}; // Artist variable from ratedlib.dsv
    std::string currentArtistInterval; // token is the contents of each column of data
    std::string currentArtist; // Artist variable from artistsadj.txt
    std::string tokenLTP;
    std::string delim{"^"};
    std::string selectedArtistToken; // Artist variable from ratedlib.dsv
    std::string ratingCode;
    std::string songLength;
    std::string artistInterval;

    //std::map<double,std::string> countMap; // Create a map for two types, string and double
    std::vector<std::string>histvect;

    // Outer loop: iterate through ratedSongsTable in the file "rated.dsv" t
    while (std::getline(ratedSongsTable, str1))
    {  // Declare variables applicable to all rows
        std::istringstream iss(str1); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str

        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, '^'))
        {
            // TOKEN PROCESSING - COL 1
            if ((tokenCount == 1) && (customArtistID == false)) {selectedArtistToken = token;}// if 'non-custom' artist is selected use this code

            // TOKEN PROCESSING - COL 12
            if (tokenCount == 12)
            {songLength = token;}// store rating variable

            // TOKEN PROCESSING - COL 17
            if ((tokenCount == 17) && (token != "0"))
            {tokenLTP = token;}// get LastPlayed Date in SQL Time

            // TOKEN PROCESSING - COL 18
            if ((tokenCount == 18) && (token == "0"))
            {notInPlaylist = true;}// store temp variable to check that item is not in playlist

            // TOKEN PROCESSING - COL 19
            if ((tokenCount == 19) && (customArtistID == true)) {selectedArtistToken = token;}// if custom artist grouping is selected use this code

            // TOKEN PROCESSING - COL 29
            if (tokenCount == 29)
            {ratingCode = token;}// store rating variable

            // TOKEN PROCESSING - COL 36
            if (tokenCount == 36)
            {artistInterval = token;}// store artistInterval variable

            ++ tokenCount;
        }
        if (notInPlaylist == true) {histvect.push_back(tokenLTP+","+selectedArtistToken+","+songLength+","+ratingCode+","+artistInterval);
            //playlistPosList << tokenLTP << "," << selectedArtistToken << std::endl;
            //std::cout << tokenLTP << "," << selectedArtistToken << std::endl;
            continue;}
    }
    std::sort (histvect.begin(), histvect.end());
    std::reverse (histvect.begin(), histvect.end());

    std::vector<std::string>::const_iterator begin = histvect.begin();
    std::vector<std::string>::const_iterator last = histvect.begin() + *_shistCount;
    std::vector<std::string> new_histvect(begin, last);
    //std::cout << "Size: "<< new_histvect.size() << std::endl;


    for (std::size_t i = 0 ;  i < new_histvect.size(); i++){
        playlistPosList << new_histvect[i] << "," << i + 1 + *_splaylistSize << "\n";}

// now just need to number each element sequentially starting with playlist size +1, then compare number and artist of extended list to
    // ratedlib.dsv then add to excluded artists list, removing duplicates.
    histvect.shrink_to_fit();
    new_histvect.shrink_to_fit();
    ratedSongsTable.close();
    playlistPosList.close();
}


