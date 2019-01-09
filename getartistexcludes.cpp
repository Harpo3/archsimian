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
    filestrinterval.open ("ratedlib.dsv");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "getArtistExcludes: Error opening rated.dsv file after it was created in child process." << std::endl;}
    std::string ratedlibrary = "ratedlib.dsv"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "getArtistExcludes: Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream playlistPosList("playlistposlist.txt"); // output file for writing rated.dsv with added artist intervals

    std::string str1; // store the string for ratedlib.dsv
    std::string playlistPosition; // Custom1 variable from ratedlib.dsv
    std::string selectedArtistToken; // Artist variable from ratedlib.dsv

    std::map<std::string,int> countMap; // Create a map for two types, string and int
    std::vector<std::string>plvect;
    plvect.reserve(5000);

    // Outer loop: iterate through ratedSongsTable in the file "ratedlib.dsv" then use str1 to add Col 36 to ratedlib.dsv
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
        if ((playlistPosition != "0") && (playlistPosition != "Custom1")){plvect.push_back(selectedArtistToken+','+playlistPosition);}

        playlistPosition = "0";
    }
    std::sort (plvect.begin(), plvect.end());
    for(unsigned long i = 0; i < plvect.size() ; i++)
    {
        playlistPosList << plvect.at(i) << "\n";
    }
    plvect.shrink_to_fit();
    ratedSongsTable.close(); // Close rated.dsv and output file
    playlistPosList.close(); 
}
