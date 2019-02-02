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
    std::fstream filestrinterval;
    filestrinterval.open ("ratedabbr.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "getArtistExcludes: Error opening ratedabbr.txt file after it was created in child process." << std::endl;}
    std::string ratedlibrary = "ratedabbr.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "getArtistExcludes: Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream playlistPosList("playlistposlist.txt"); // output file for writing ratedabbr.txt with added artist intervals

    std::string str1; // store the string for ratedabbr.txt
    std::string playlistPosition; // Custom1 variable from ratedabbr.txt
    std::string selectedArtistToken; // Artist variable from ratedabbr.txt

    std::map<std::string,int> countMap; // Create a map for two types, string and int
    std::vector<std::string>plvect;
    plvect.reserve(5000);
    // Outer loop: iterate through ratedSongsTable in the file "ratedabbr.txt" then use str1 to add Col 36 to ratedabbr.txt
    while (std::getline(ratedSongsTable, str1))
    {  // Declare variables applicable to all rows
        std::istringstream iss(str1); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, ','))
        {
            // TOKEN PROCESSING - COL 1
            if ((tokenCount == 1) && (token != "0"))
            {playlistPosition = token;} // playlist position, exclude if not in playlist
            // TOKEN PROCESSING - COL 3
            if (tokenCount == 3)  {selectedArtistToken = token;}//  artist is selected
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
    ratedSongsTable.close(); // Close ratedabbr.txt and output file
    playlistPosList.close();
}
