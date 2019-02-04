#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <map>
#include <stdio.h>
#include "basiclibfunctions.h"
#include "getplaylist.h"
#include "constants.h"

//using namespace std;

// One must be very careful when comparing floating point numbers for equality
// so instead, use this function to get 'close enough' to a match:
inline bool isEqual(double x, double y){
    const double epsilon =  1e-5;
    return std::abs(x - y) <= epsilon * std::abs(x);
    // see http://www.cs.technion.ac.il/users/yechiel/c++-faq/floating-point-arith.html
}

void trim_cruft(std::string& buffer)
{
    static const char cruft[] = "\n\r";
    buffer.erase(buffer.find_last_not_of(cruft) + 1);
}

void getExcludedArtists(long *_shistCount, int *_splaylistSize)
{
    std::fstream filestrinterval;
    int s_playlistPosition;
    filestrinterval.open ("ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "getArtistExcludes: Error opening ratedabbr.txt file ." << std::endl;}
    std::string ratedlibrary = "ratedabbr.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "getArtistExcludes: Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    StringVector2D ratedabbrVec = readCSV("ratedabbr.txt");
    std::vector<std::string>histvect;
    std::vector<std::string> artistExcludesVec;
    std::ofstream playlistPosList("playlistposlist.txt"); // output file for writing ratedabbr.txt with added artist intervals
    std::ofstream ratedabbr2("ratedabbr2.txt"); // output file for writing ratedabbr.txt with added artist intervals
    std::string str1; // store the string for ratedabbr.txt
    //std::string playlistPosition; // Custom1 variable for playlistposlist.txt
    std::string selectedArtistToken; // Artist variable from ratedabbrVec
    std::string tokenLTP; // LastPlayed Date in SQL Time from ratedabbrVec
    std::string ratingCode; // Rating code from ratedabbrVec
    std::string songLength; // Song length from ratedabbrVec
    std::string artistInterval; // Artist interval from ratedabbrVec
    std::string path;
    std::map<std::string,int> countMap; // Create a map for two types, string and int
    std::vector<std::string>plvect;
    plvect.reserve(5000);

    //open playlist to read in position number of each track
    std::fstream playList;
    playList.open ("cleanedplaylist.txt");
    if (playList.is_open()) {playList.close();}
    else {std::cout << "getArtistExcludes: Error opening cleanedplaylist.txt file." << std::endl;}
    std::string playlist = "cleanedplaylist.txt"; // now we can use it as input file
    //*_splaylistSize = cstyleStringCount("cleanedplaylist.txt");


    //std::cout << "getExcludedArtists: *_splaylistSize is: "<<*_splaylistSize << std::endl;


    std::ifstream playlistTable(playlist);
    if (!playlistTable.is_open())
    {
        std::cout << "getArtistExcludes: Error opening playlistTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    int playlistCount = 0;
    int songCount = 0;
    static std::string song;
    while (getline(playlistTable,song)){
        ++playlistCount;
        s_playlistPosition = *_splaylistSize-playlistCount + 1; // playlist position is most recently added (to bottom)
        std::string txtPos;
        std::string pathinlib;
        std::string artistinvec;
        txtPos = std::to_string(s_playlistPosition);
        //std::cout << " PL POS: "<<s_playlistPosition  << " " << song <<std::endl;
        for(size_t i = 0; i < ratedabbrVec.size(); i++){ // read each row element into the variables needed
            tokenLTP = ratedabbrVec[i][0];
            ratingCode = ratedabbrVec[i][1];
            selectedArtistToken = ratedabbrVec[i][2];
            pathinlib = ratedabbrVec[i][3];
            songLength = ratedabbrVec[i][4];
            artistInterval = ratedabbrVec[i][5];
            trim_cruft(pathinlib);
            trim_cruft(song);
            pathinlib.compare(song);
            // histvect - if path in vector row does not match a playlist entry, push to extended
            // play history vector
            if ((song != pathinlib) && (songCount == 1)){
                histvect.push_back(tokenLTP+","+selectedArtistToken+","+songLength+","+ratingCode+","+artistInterval);
                continue;
            }
            // ratedabbrVec - if path in vector row matches a playlist entry, remove zero from col 6 and emplace
            // playlist position number
            if (song == pathinlib){
                for(int j=0;j<20;j++){
                    ratedabbrVec[i].pop_back();
                    ratedabbrVec[i].emplace_back(txtPos);
                }
            }
            if (song == pathinlib){
                //compare playlist position to size of playlist and interval
                 if (s_playlistPosition < std::stoi(artistInterval)){ //add artist to exclude list
                    artistExcludesVec.push_back(selectedArtistToken);}
                }
            ++songCount;
        }
    }
    std::sort (histvect.begin(), histvect.end());
    std::reverse (histvect.begin(), histvect.end());

    std::string strbuild;
    std::sort (ratedabbrVec.begin(), ratedabbrVec.end());
    //
    // add code here to populate excludedartists vector using col 6 of ratedabbrVec
    // for playlist positions emplaced from playlist, and extended positions emplaced by histvect
    //

    // Writes a 2D vector of strings to a file, each row with comma separated values
    // This file (ratedabbr2.txt) will be later used to select next track for playlist (total rating times, and current playlist position)
    for ( const auto &row : ratedabbrVec )
    {
        bool first = true;
        for ( const auto &s : row ) {
        if (first == true){ratedabbr2 << s;}
        else {ratedabbr2 <<','<< s;}
        first = false;
        }
        ratedabbr2 << std::endl;
    }

    std::vector<std::string>::const_iterator begin = histvect.begin();
    std::vector<std::string>::const_iterator last = histvect.begin() + *_shistCount;
    std::vector<std::string> new_histvect(begin, last); // copy histvect to new vector
    for (std::size_t i = 0 ;  i < new_histvect.size(); i++){ // write new vector to "playlistposlist.txt"
        playlistPosList << new_histvect[i] << "," << i + 1 + *_splaylistSize << "\n";
    }
    playlistPosList.close();
    // Calculate the extended playlist to find infrequent artists played within their interval value (ex. postion 452 and interval 487)
    std::ofstream artistExcList("artistexcludes.txt"); // output file for writing final exclude list
    StringVector2D finalhistvec = readCSV("playlistposlist.txt"); // open "playlistposlist.txt" as 2D vector finalhistvec
    for (std::size_t i = 0 ;  i < finalhistvec.size(); i++){
        for(int j=0;j<6;j++){
            //std::cout << finalhistvec[i][4] << ", " <<finalhistvec[i][5] << std::endl;
            if (std::stoi(finalhistvec[i][4]) > std::stoi(finalhistvec[i][5])){ // if playlist pos [6] is less than interval [5],
                //artistExcList << finalhistvec[i][1] << std::endl;
                //std::cout << finalhistvec[i][1] << ", "<< finalhistvec[i][4] << ", " <<finalhistvec[i][5]<< std::endl;
                artistExcludesVec.push_back(finalhistvec[i][1]); // then pushback artist (finalhistvec[i][1]) to artistExcludesVec
            }
        }
    }
    // Compile all of the entries of the artistExcludesVec, sort and remove duplicates, then write the vector to the excluded artists file
    std::vector<std::string>fsexclvec;
    fsexclvec.reserve(10000);
    fsexclvec = artistExcludesVec;
    std::vector<std::string>::iterator ip;
    std::sort (fsexclvec.begin(), fsexclvec.end());
    size_t myvecsz = fsexclvec.size();
    ip = std::unique(fsexclvec.begin(), fsexclvec.begin() + myvecsz); // 500 here is max number of unique artists - need to set variable
    fsexclvec.resize(std::distance(fsexclvec.begin(), ip));
    for (ip = fsexclvec.begin(); ip != fsexclvec.end(); ++ip) {
        artistExcList << *ip << "\n";
    }
    ratedSongsTable.close(); // Close ratedabbr.txt and output file
    ratedabbr2.close();
    artistExcList.close();
    //remove("playlistposlist.txt");
}

int ratingCodeSelected(double *_sratingRatio3, double *_sratingRatio4, double *_sratingRatio5,
                       double *_sratingRatio6, double *_sratingRatio7, double *_sratingRatio8){
    //Lookup the rating codes for last two tracks on the playlist;
    //std::cout << "ratingCodeSelected started." << std::endl;
    int x = 0; // variable to return the rating code to be used for the next track selection
    std::string codeForPos1;
    std::string codeForPos2;
    bool exclude7and8 = false;
    std::string codeForPos;
    //Collect the time on the current playlist for each rating category and for the playlist as a whole;
    //variables:
    double totalPlaylistTime{0};
    double totalPLTime1{0};
    double totalPLTime3{0};
    double totalPLTime4{0};
    double totalPLTime5{0};
    double totalPLTime6{0};
    double totalPLTime7{0};
    double totalPLTime8{0};
    // Iterate through ratedabbr2.txt and increment the time for each rating code and total for playlist
    // COL 4 - SongLength
    // COL 1 - PlaylistPosition
    // COL 2 - RatingCode
    //variables:
    std::string str; // store the string for ratedabbr2.txt
    double selectedSongLength{0.0};
    std::string selectedPlaylistPosition;
    std::string selectedRatingCode;
    std::fstream filestrinterval;
    filestrinterval.open ("ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "ratingCodeSelected: Error opening ratedabbr2.txt file." << std::endl;}
    std::string ratedlibrary = "ratedabbr2.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()) {
        std::cout << "ratingCodeSelected: Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    while (std::getline(ratedSongsTable, str)) { // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, ',')) {
            // TOKEN PROCESSING - COL 1
            if (tokenCount == 1)  {
                selectedRatingCode = token;
                //if (selectedPlaylistPosition == "1") {codeForPos1 = selectedRatingCode;
                //std::cout << "selectedPlaylistPosition 1 should be: "<< str <<" rating: " <<token << std::endl;
                //}
                //if (selectedPlaylistPosition == "2") {codeForPos2 = selectedRatingCode;
                 //   std::cout << "selectedPlaylistPosition 2 should be: "<< str <<" rating: " <<token << std::endl;
               // }
            }
            // TOKEN PROCESSING - COL 4
            if (tokenCount == 4)  {selectedSongLength = std::atof(token.c_str());
            }
            // TOKEN PROCESSING - COL 6
            if (tokenCount == 6)  {
                selectedPlaylistPosition = token;
                if (token == "1") {
                    codeForPos1 = selectedRatingCode;
                    if (Constants::verbose == true)std::cout << "selectedPlaylistPosition 1 is: "<< str << std::endl;
                }
                if (token == "2") {
                    codeForPos2 = selectedRatingCode;
                    if (Constants::verbose == true)std::cout << "selectedPlaylistPosition 2 is: "<< str << std::endl;
                }
            }
            ++ tokenCount;
        }
        if (selectedPlaylistPosition != "0") {
            totalPlaylistTime = totalPlaylistTime + selectedSongLength;
            if (selectedRatingCode == "1") {totalPLTime1 = totalPLTime1 + selectedSongLength;}
            if (selectedRatingCode == "3") {totalPLTime3 = totalPLTime3 + selectedSongLength;}
            if (selectedRatingCode == "4") {totalPLTime4 = totalPLTime4 + selectedSongLength;}
            if (selectedRatingCode == "5") {totalPLTime5 = totalPLTime5 + selectedSongLength;}
            if (selectedRatingCode == "6") {totalPLTime6 = totalPLTime6 + selectedSongLength;}
            if (selectedRatingCode == "7") {totalPLTime7 = totalPLTime7 + selectedSongLength;}
            if (selectedRatingCode == "8") {totalPLTime8 = totalPLTime8 + selectedSongLength;}
        }
    }
    if (Constants::verbose == true) {
    std::cout << "totalPLTime3 is: " <<totalPLTime3/1000 << std::endl;
    std::cout << "totalPLTime4 is: " <<totalPLTime4/1000 << std::endl;
    std::cout << "totalPLTime5 is: " <<totalPLTime5/1000 << std::endl;
    std::cout << "totalPLTime6 is: " <<totalPLTime6/1000 << std::endl;
    std::cout << "totalPLTime7 is: " <<totalPLTime7/1000 << std::endl;
    std::cout << "totalPLTime8 is: " <<totalPLTime8/1000 << std::endl;
    std::cout << "totalPlaylistTime is: " <<totalPlaylistTime/1000 << std::endl;
    }

    //Calculate time ratio for each rating code by dividing each by the total playlist time.
    //variables:
    double ratioTime3 = totalPLTime3 / totalPlaylistTime;
    double ratioTime4 = totalPLTime4 / totalPlaylistTime;
    double ratioTime5 = totalPLTime5 / totalPlaylistTime;
    double ratioTime6 = totalPLTime6 / totalPlaylistTime;
    double ratioTime7 = totalPLTime7 / totalPlaylistTime;
    double ratioTime8 = totalPLTime8 / totalPlaylistTime;
    if (Constants::verbose == true) {
    std::cout << "RatioTime3 is: " << ratioTime3 << " versus std: " << *_sratingRatio3 << std::endl;
    std::cout << "RatioTime4 is: " << ratioTime4 << " versus std: " << *_sratingRatio4<< std::endl;
    std::cout << "RatioTime5 is: " << ratioTime5 << " versus std: " << *_sratingRatio5<< std::endl;
    std::cout << "RatioTime6 is: " << ratioTime6 << " versus std: " << *_sratingRatio6<< std::endl;
    std::cout << "RatioTime7 is: " << ratioTime7 << " versus std: " << *_sratingRatio7<< std::endl;
    std::cout << "RatioTime8 is: " << ratioTime8 << " versus std: " << *_sratingRatio8<< std::endl;
    }
    //Compare the ratio for each rating code on the playlist to the rating code standards set by the program.
    //variables:
    double varianceRatioTime3 = (*_sratingRatio3 - ratioTime3) / *_sratingRatio3;
    double varianceRatioTime4 = (*_sratingRatio4 - ratioTime4) / *_sratingRatio4;
    double varianceRatioTime5 = (*_sratingRatio5 - ratioTime5) / *_sratingRatio5;
    double varianceRatioTime6 = (*_sratingRatio6 - ratioTime6) / *_sratingRatio6;
    double varianceRatioTime7 = (*_sratingRatio7 - ratioTime7) / *_sratingRatio7;
    double varianceRatioTime8 = (*_sratingRatio8 - ratioTime8) / *_sratingRatio8;
    if (Constants::verbose == true) {
    std::cout << "varianceRatioTime3 is: " << varianceRatioTime3 << std::endl;
    std::cout << "varianceRatioTime4 is: " << varianceRatioTime4 << std::endl;
    std::cout << "varianceRatioTime5 is: " << varianceRatioTime5 << std::endl;
    std::cout << "varianceRatioTime6 is: " << varianceRatioTime6 << std::endl;
    std::cout << "varianceRatioTime7 is: " << varianceRatioTime7 << std::endl;
    std::cout << "varianceRatioTime8 is: " << varianceRatioTime8 << std::endl;
    std::cout << "Rating for last track added was: " <<codeForPos1;
    std::cout << ", and second-to-last was: " <<codeForPos2 << std::endl;
    }
    //If the playlist ratio is less than the program ratio, then that code is underrepresented on the playlist.
    // The largest positive number is the most underrepresented rating code of the current playlist.
    // Create constants with the variances just calculated for each rating code variance to place them in an array
    // Array a_variances can now contain the const variance values
    const double vrt3 = varianceRatioTime3;
    const double vrt4 = varianceRatioTime4;
    const double vrt5 = varianceRatioTime5;
    const double vrt6 = varianceRatioTime6;
    const double vrt7 = varianceRatioTime7;
    const double vrt8 = varianceRatioTime8;
    // Determine whether a code of 7 or 8 was added in either of the last two tracks
    if ((codeForPos1 == "7") || (codeForPos1 == "8") || (codeForPos2 == "7") || (codeForPos2 == "8")) {
        exclude7and8 = true;
    }
    else {exclude7and8 = false;}
    //
    // Condition 1
    // If both of the last two tracks was a code 7 or 8 (should not occur), exclude from consideration
    if (((codeForPos1=="7") || (codeForPos1=="8")) && ((codeForPos2 == "7") || (codeForPos2 =="8"))) {
        double a_variances[] = {vrt3, vrt4, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 4);
        if (Constants::verbose == true) std::cout << "Condition 1. Evaluating codes 3, 4, 5, 6." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
    }
    // Condition 1a
    // If the last track was either a code 7 or 8, exclude 7 and 8 from consideration
    if ((codeForPos1=="7") || (codeForPos1=="8")) {
        double a_variances[] = {vrt3, vrt4, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 4);
        if (Constants::verbose == true) std::cout << "Condition 1a. Evaluating codes 3, 4, 5, 6." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
    }
    // Condition 2
    // If the second to last track was a code 7 or 8, and last track was a 3, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  && (codeForPos1=="3")) {
        double a_variances[] = {vrt4, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        if (Constants::verbose == true) std::cout << "Condition 2. Evaluating codes 4, 5, 6." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
    }
    // Condition 3
    // If the second to last track was a code 7 or 8, and last track was a 4, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  &&  (codeForPos1=="4")) {
        double a_variances[] = {vrt3, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        if (Constants::verbose == true) std::cout << "Condition 3. Evaluating codes 3, 5, 6." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
    }
    // Condition 4
    // If the second to last track was a code 7 or 8, and last track was a 5, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  && (codeForPos1=="5")) {
        double a_variances[] = {vrt3, vrt4, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        if (Constants::verbose == true) std::cout << "Condition 4. Evaluating codes 3, 4, 6." << std::endl;;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
    }
    // Condition 5
    // If the second to last track was a code 7 or 8, and last track was a 6, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  && (codeForPos1=="6")) {
        double a_variances[] = {vrt3, vrt4, vrt5};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        if (Constants::verbose == true) std::cout << "Condition 5. Evaluating codes 3, 4, 5." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
    }
    // Condition 6
    // If neither of the last two tracks was a code 7 or 8, and last track was a 3, exclude from consideration
    if ((exclude7and8==false) && (codeForPos1=="3")) {
        double a_variances[] = {vrt4, vrt5, vrt6, vrt7, vrt8};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 5);
        if (Constants::verbose == true) std::cout << "Condition 6. Evaluating codes 4, 5, 6, 7, 8." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
        if (isEqual(*maxVariance,varianceRatioTime7)== 1) {x = 7;}
        if (isEqual(*maxVariance,varianceRatioTime8)== 1) {x = 8;}
    }
    // Condition 7
    // If neither of the last two tracks was a code 7 or 8, and last track was a 4, exclude from consideration
    if ((exclude7and8==false) && (codeForPos1=="4")) {
        double a_variances[] = {vrt3, vrt5, vrt6, vrt7, vrt8};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 5);
        if (Constants::verbose == true) std::cout << "Condition 7. Evaluating codes 3, 5, 6, 7, 8." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
        if (isEqual(*maxVariance,varianceRatioTime7)== 1) {x = 7;}
        if (isEqual(*maxVariance,varianceRatioTime8)== 1) {x = 8;}
    }
    // Condition 8
    // If neither of the last two tracks was a code 7 or 8, and last track was a 5, exclude from consideration
    if ((exclude7and8==false) && (codeForPos1=="5")) {
        double a_variances[] = {vrt3, vrt4, vrt6, vrt7, vrt8};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 5);
        if (Constants::verbose == true) std::cout << "Condition 8. Evaluating codes 3, 4, 6, 7, 8." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
        if (isEqual(*maxVariance,varianceRatioTime7)== 1) {x = 7;}
        if (isEqual(*maxVariance,varianceRatioTime8)== 1) {x = 8;}
    }
    // Condition 9
    // If neither of the last two tracks was a code 7 or 8, and last track was a 6, exclude from consideration
    if ((exclude7and8==false) && (codeForPos1=="6")) {
        double a_variances[] = {vrt3, vrt4, vrt5, vrt7, vrt8};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 5);
        if (Constants::verbose == true) std::cout << "Condition 9. Evaluating codes 3, 4, 5, 7, 8." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime7)== 1) {x = 7;}
        if (isEqual(*maxVariance,varianceRatioTime8)== 1) {x = 8;}
    }
    ratedSongsTable.close();
    return x;
}

std::vector<std::string> split(std::string strToSplit, char delimeter){
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter)) {
        splittedStrings.push_back(item);
    }
    return splittedStrings;
}
//Screen out tracks in ratedabbr2.txt that do not match the rating of the s_ratingNextTrack variable
// Screen out tracks that have an artist that matches any when iterating through the artistexcludes.txt file
// Send remaining to a new vector
// Sort vector to select the oldest dated track for addition to the playlist
// Write/append the cleanedplaylist.txt file the oldest dated track found.

//std::ofstream outfratedint2("extendexcludes.txt"); // output file for writing ratedabbr2.txt with added artist intervals

void selectTrack(int *_sratingNextTrack){
    if (Constants::verbose == true) std::cout << "Starting selectTrack function. Rating for next track is " << *_sratingNextTrack << std::endl;
    std::fstream filestrinterval;
    filestrinterval.open ("ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "Error opening ratedabbr2.txt file." << std::endl;}
    std::string ratedlibrary = "ratedabbr2.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()) {
        std::cout << "selectTrack: Error opening ratedabbr2.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for ratedabbr2.txt
    std::string str2; // store the string for artistexcludes.txt
    std::string str3; // store the string for ratedabbr2.txt (second opening)
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
    if (Constants::verbose == true) std::cout << "selectTrack function: Created new vector to store final selections" << std::endl;
    // Outer loop: iterate through ratedSongsTable in the file "ratedabbr2.txt"
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
            if (tokenCount == 1) {ratingCode = token;}// store rating variable
            // TOKEN PROCESSING - COL 2
            if (tokenCount == 2) {selectedArtistToken = token;} //Selected artist token
            // TOKEN PROCESSING - COL 3
            if (tokenCount == 3) {songPath = token;}// store song path variable
            // TOKEN PROCESSING - COL 4
            if (tokenCount == 4) {songLengtha = token;} //just added
            // TOKEN PROCESSING - COL 5
            if (tokenCount == 5) {artistIntervala = token;} //just added
            // TOKEN PROCESSING - COL 6
            if (tokenCount == 6)  {playlistPos = token;}
            ++ tokenCount;
        }
        if (playlistPos == "0") {notInPlaylist = 1;}
        else if (playlistPos != "0"){notInPlaylist = 0;}// store temp variable to check that item is not in playlist
        //std::cout << "Artist: " << selectedArtistToken;
        // For either of these variable results, continue to next row: (1) rating is not equal to s_ratingNextTrack (2) item is already on the
        // playlist, notInPlaylist == false
        if (notInPlaylist == 0) {continue;}
        //std::cout << "notInPlaylist = false:" << notInPlaylist << ". Going to next line..." << std::endl;
        else if (ratingCode != std::to_string(*_sratingNextTrack)) {continue;}
        //std::cout << ". RatingCode is: " << ratingCode << " and *_sratingNextTrack is " << *_sratingNextTrack << ". Going to next line..." << std::endl;
        else if ((ratingCode == std::to_string(*_sratingNextTrack)) && (notInPlaylist == 1)){
            //std::cout << "Track found with current rating (not yet excluded): " << selectedArtistToken << ", " << songPath << std::endl;
        }
        // If not yet skipped, open an inner loop and iterate through artistexcludes.txt and compare each entry against the artist token.
        // Continue to next row if a match found.
        //std::cout << "selectTrack function: ifstream artistexcludes." << std::endl;
        std::ifstream artistexcludes;  // Next ensure artistexcludes.txt is ready to open
        artistexcludes.open ("artistexcludes.txt");
        if (artistexcludes.is_open()) {artistexcludes.close();}
        else {std::cout << "Error opening artistexcludes.txt file." << std::endl;}
        std::string artistexcludes2 = "artistexcludes.txt"; // now we can use it as input file
        std::ifstream artexcludes(artistexcludes2); // Open artistexcludes.txt as ifstream
        if (!artexcludes.is_open()) {
            std::cout << "Error opening artistexcludes.txt." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        s_excludeMatch = false;
        while (std::getline(artexcludes, str2)) {
            //std::cout << "Artist from artistexcludes.txt: "<< str2<<". Artist with sel rating from ratedabbr2.txt is: "<<selectedArtistToken << std::endl;
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
    if (Constants::verbose == true) std::cout << "selectTrack function: Write/append s_selectedTrackPath to the cleanedplaylist.txt file." << std::endl;
    //Write/append s_selectedTrackPath to the cleanedplaylist.txt file.
    std::ofstream playlist("cleanedplaylist.txt",std::ios::app);
    playlist << selectedTrackPath << "\n";
    playlist.close();
    ratedSongsTable.close();
    if (Constants::verbose == true) std::cout <<'\n';
    std::cout << "Added "<< selectedTrackPath;
    //remove("ratedabbr2.txt");
    //ratedabbrvec.shrink_to_fit();
    finaltracksvect.shrink_to_fit();
}
