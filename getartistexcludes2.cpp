#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdio.h>

void getArtistExcludes2()
{
    std::vector<std::string>pexclvec;
    pexclvec.reserve(50000);
    std::ofstream outfratedint("artistexcludes.txt"); // output file for writing
    int artStd{0}; // token is the contents of each column of data
    std::string currentArtist2; // Artist variable from artistsadj.txt

    std::string str3; // store the string for artistadj.txt
    std::string str4; // store the string for playlistposlist.txt

    // Open artistadj.txt and compare interval of each artist to playlist positions shown in playlistposlist.txt.
    //If interval value in artistadj.txt is less than playlist position(s) in playlistposlist.txt, add artist to excluded list.
    std::fstream artistadj;
    artistadj.open ("artistsadj.txt");
    if (artistadj.is_open()) {artistadj.close();}
    else {std::cout << "getArtistExcludes2: Error opening artistadj.txt file." << std::endl;}
    std::string intervalStd = "artistsadj.txt"; // now we can use it as input file
    std::ifstream artistIntStds(intervalStd);
    if (!artistIntStds.is_open())
    {
        std::cout << "getArtistExcludes2: Error opening artistIntStds." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    // Iterate through artistadj.txt and store int value of col 4 to variable artStd

    // Outer loop: iterate through the file "artistadj.txt"
    while (std::getline(artistIntStds, str3))
    {  // Declare variables applicable to all rows
        std::istringstream iss(str3); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        int plIntervalVal{0};
        std::string selectedArtistToken2;

        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, ','))
        {
            // TOKEN PROCESSING - COL 0
            if (tokenCount == 0) {selectedArtistToken2 = token;}
            // TOKEN PROCESSING - COL 4
            if (tokenCount == 4) {artStd = std::stoi(token);}
            ++ tokenCount;
        }

        // While on row of artistadj.txt, iterate through playlistposlist.txt and save each Col 4 to int variable plIntervalVal

        // First loop: iterate through each column (token) of current row of playlistposlist.txt
        // to get the artist name and interval to write interval to ratedlib.dsv
        std::fstream artistPLIntervals;  // Next ensure artistsadj.txt is ready to open
        artistPLIntervals.open ("playlistposlist.txt");
        int playlistPosNum{0};
        if (artistPLIntervals.is_open()) {artistPLIntervals.close();}
        else {std::cout << "getArtistExcludes2: Error opening artistsadj.txt file." << std::endl;}
        std::string artistIntPL = "playlistposlist.txt"; // now we can use it as input file
        std::ifstream plPosList(artistIntPL); // Open playlistposlist.txt as ifstream
        if (!plPosList.is_open())
        {
            std::cout << "getArtistExcludes2: Error opening playlistposlist.txt." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        while (std::getline(plPosList, str4))
        {
            std::istringstream iss(str4); // str is the string of each row
            int tokenPLCount{0}; //token count is the number of delimiter characters within str
            std::string token; // token is the contents of each column of data
            while (std::getline(iss, token, ','))
            {
                // TOKEN PROCESSING - COL 0
                if (tokenPLCount == 0) {currentArtist2 = token;}

                // TOKEN PROCESSING - COL 1
                if (tokenPLCount == 1) {playlistPosNum = std::stoi(token);}
                //  }
                ++tokenPLCount;
            }
            // If plIntervalVal < artStd  write artist to artistexcludes.txt
            if ((selectedArtistToken2 == currentArtist2) && (playlistPosNum < artStd)){
                // push unique values into vector pexclvec
                pexclvec.push_back(currentArtist2);
                //std::cout << "getArtistExcludes2: Found artist to exclude: " <<selectedArtistToken2<<". Playlist pos: " <<std::to_string(playlistPosNum)<< " and std: " <<artStd << std::endl;
                continue; // Resume artistadj.txt next row, beginning with Col 0
            }
            continue; // Resume artistadj.txt next row, beginning with Col 0
        }
        plPosList.close();
    }
    std::vector<std::string>::iterator ip;
    std::sort (pexclvec.begin(), pexclvec.end());
    size_t myvecsz = pexclvec.size();
    ip = std::unique(pexclvec.begin(), pexclvec.begin() + myvecsz);
    pexclvec.resize(std::distance(pexclvec.begin(), ip));

    for (ip = pexclvec.begin(); ip != pexclvec.end(); ++ip) {
        outfratedint << *ip << "\n";
    }    
    artistIntStds.close();
    outfratedint.close();
    pexclvec.shrink_to_fit();
}
