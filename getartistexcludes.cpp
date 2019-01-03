#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdio.h>

void getArtistExcludes()
{
    bool customArtistID{true};
    std::fstream filestrinterval;
    filestrinterval.open ("rated2.dsv");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "Error opening rated.dsv file after it was created in child process." << std::endl;}
    std::string ratedlibrary = "rated2.dsv"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream playlistPosList("playlistposlist.txt"); // output file for writing rated.dsv with added artist intervals

    std::string str1; // store the string for rated2.dsv
    std::string playlistPosition; // Custom1 variable from rated2.dsv
    std::string selectedArtistToken; // Artist variable from rated2.dsv

    std::string currentArtistInterval; // token is the contents of each column of data
    std::string currentArtist; // Artist variable from artistsadj.txt
    std::string delim{"^"};

    std::map<std::string,int> countMap; // Create a map for two types, string and int
    std::vector<std::string>plvect;

    // Outer loop: iterate through ratedSongsTable in the file "rated2.dsv" then use str1 to add Col 36 to rated2.dsv
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

            // TOKEN PROCESSING - COL 18
            if ((tokenCount == 18) && (token != "0"))
            {playlistPosition = token;}// if 'non-custom' artist is selected use this code
            //if custom artist grouping is selected use this code

            // TOKEN PROCESSING - COL 19
            if ((tokenCount == 19) && (customArtistID == true) && (playlistPosition != "0")) {selectedArtistToken = token;}// if custom artist grouping is selected use this code

            ++ tokenCount;
        }
        //if (playlistPosition != "0")std::cout <<playlistPosition<< "," <<selectedArtistToken<< std::endl;
        if ((playlistPosition != "0") && (playlistPosition != "Custom1")){plvect.push_back(selectedArtistToken+','+playlistPosition);}

        playlistPosition = "0";
    }
    std::sort (plvect.begin(), plvect.end());
    for(unsigned long i = 0; i < plvect.size() ; i++)
    {
        playlistPosList << plvect.at(i) << std::endl;
    }
    ratedSongsTable.close(); // Close rated.dsv and output file
    playlistPosList.close(); 
}
