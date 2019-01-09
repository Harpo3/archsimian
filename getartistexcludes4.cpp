#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdio.h>

void appendExcludes()
{
    std::ifstream extendlist;
    extendlist.open ("histposlist.txt");
    if (extendlist.is_open()) {extendlist.close();}
    else {std::cout << "Error opening histposlist.txt file." << std::endl;}
    std::string exthistory = "histposlist.txt"; // now we can use it as input file
    std::ifstream exthistoryTable(exthistory);
    if (!exthistoryTable.is_open())
    {
        std::cout << "appendExcludes: Error opening exthistoryTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    // Append write to excludes list, then remove duplicates from it

    std::ofstream artistExcList("artistexcludes.txt",std::ios::app); // output file for writing history not in playlist for longer artist intervals
    // If we couldn't open the output file stream for writing
    if (!artistExcList)
    {
        // Print an error and exit
        std::cerr << "appendExcludes: Uh oh, artistexcludes.txt could not be opened for writing!" << std::endl;
        exit(1);
    }
    std::string str; // store the string for histposlist.txt
    std::string selectedArtistToken; // Artist variable from histposlist.txt
    std::string currentArtistInterval; // token is the contents of each column of data
    std::string currentArtist; // Artist variable from artistsadj.txt
    std::string posExtList; // Playlist position in extended list

    // Outer loop: iterate through ratedSongsTable in the file "rated.dsv" t
    while (std::getline(exthistoryTable, str))
    {  // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str

        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, ','))
        {
            // TOKEN PROCESSING - COL 1 Artist Name
            if (tokenCount == 1)  {selectedArtistToken = token;}
            // TOKEN PROCESSING - COL 4 Artist Interval
            if (tokenCount == 4)
            {currentArtistInterval = token;}
            // TOKEN PROCESSING - COL 5 Playlist position in extended list
            if (tokenCount == 5)
            {posExtList = token;}

            ++ tokenCount;
        }
        int selectedInterval = std::stoi(currentArtistInterval);
        int selectedPLPos = std::stoi(posExtList);
        if (selectedPLPos < selectedInterval) { // If artist's lastplayed position is less than its interval,
            artistExcList << selectedArtistToken << "\n"; // then the artist must be excluded
            continue;// Add the artist to the excluded list and go to the next entry
        }
    }
    artistExcList.close();
    exthistoryTable.close();
}
