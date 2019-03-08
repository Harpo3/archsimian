#include <fstream>
#include <sstream>
#include <vector>
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

// Compares two strings and returns bool for match result
bool stringMatch(std::string s1, std::string s2)
{
    bool x{0};
    trim_cruft(s1);
    trim_cruft(s2);
    if(s1 != s2)x = 0;
    else x = 1;
    return x;
}

// Looks for str in ifstream and returns a bool for match result
bool matchLineinIfstream(std::ifstream & stream, std::string str) {
    std::string line;
    bool x{0};
    while (getline(stream, line)) {
        x = stringMatch(line,str);
        if (x == 1) break;
    }
    return x;
}

void getExcludedArtists(const int &s_playlistSize)
{
    std::fstream filestrinterval;
    int s_playlistPosition;
    filestrinterval.open ("ratedabbr.txt");
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
    std::vector<std::string>histvect; // vector to collect excluded artists from 'outside the playlist'
    std::vector<std::string> artistExcludesVec;// vector to collect all excluded artists
    std::ofstream playlistPosList("playlistposlist.txt"); // output file for writing ratedabbr.txt with added artist intervals
    std::ofstream ratedabbr2("ratedabbr2.txt"); // output file for writing ratedabbr.txt with added artist intervals
    std::string str1; // store the string for ratedabbr.txt
    //std::string playlistPosition; // Custom1 variable for playlistposlist.txt
    std::string selectedArtistToken; // Artist variable from ratedabbrVec
    std::string tokenLTP; // LastPlayed Date in SQL Time from ratedabbrVec
    std::string ratingCode; // Rating code from ratedabbrVec
    std::string songLength; // Song length from ratedabbrVec
    std::string artistInterval; // Artist interval from ratedabbrVec
    std::string albumID; // Album ID from ratedabbrVec

    std::string path;
    //std::map<std::string,int> countMap; // Create a map for two types, string and int
    //std::vector<std::string>plvect;
    //plvect.reserve(5000);

    //open playlist to read in position number of each track
    std::fstream playList;
    playList.open (Constants::cleanedPlaylist);
    if (playList.is_open()) {playList.close();}
    else {std::cout << "getArtistExcludes: Error opening cleanedplaylist.txt file." << std::endl;}
    std::string playlist = Constants::cleanedPlaylist; // now we can use it as input file
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
        s_playlistPosition = s_playlistSize-playlistCount + 1; // playlist position is most recently added (to bottom)
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
            albumID = ratedabbrVec[i][6];
            trim_cruft(pathinlib);
            trim_cruft(song);
            pathinlib.compare(song);
            // histvect - if path in vector row does not match a playlist entry, push to extended
            // play history vector
            if ((song != pathinlib) && (songCount == 1)){
                histvect.push_back(tokenLTP+","+selectedArtistToken+","+songLength+","+ratingCode+","+artistInterval+","+albumID);
                continue;
            }
            // ratedabbrVec - if path in vector row matches a playlist entry, remove zero from col 7 and emplace
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
    std::vector<std::string> new_histvect = histvect;
    for (std::size_t i = 0 ;  i < new_histvect.size(); i++){ // write new vector to "playlistposlist.txt"
        playlistPosList << new_histvect[i] << "," << i + 1 + s_playlistSize << "\n";
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
    std::size_t myvecsz = fsexclvec.size();
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

int ratingCodeSelected(double &s_ratingRatio3, double &s_ratingRatio4, double &s_ratingRatio5,
                       double &s_ratingRatio6, double &s_ratingRatio7, double &s_ratingRatio8){
    //Lookup the rating codes for last two tracks on the playlist;
    //std::cout << "ratingCodeSelected started." << std::endl;

    int x = 0; // variable to return the rating code to be used for the next track selection
    //bool isRating1Qty = (s_rCode1TotTrackQty != 0);// set bool to true if there is at least one track with a rating of 1
   // int rating1PosCount{0};
    //std::cout << "rating Code 1 Selection started. *_srepeatFreqForCode1 result is: " <<*_repeatFreqCode1 <<  std::endl;
   // if (isRating1Qty == true) //Before going to logic for other rating codes, determine whether to select rating code 1
    //{
        // check if rating 1 total in playlist is equal to *_srCode1TotTrackQty; if true
        // set isRating1Qty to false and continue function
        // Else calculate the 'most recent' playlist position and set variable rating1PosCountuse
        // Compare rating1PosCount with *_srepeatFreqForCode1
        //bool select1Here{0};
       // select1Here = (rating1PosCount == repeatFreqCode1);
        //std::cout << "rating Code 1 Selection started. bool isRating1Qty result is: "<< isRating1Qty <<" and select1Here result is: "<<select1Here<< std::endl;
       // rating1PosCount = 1;
    //}
    std::string codeForPos1;
    std::string codeForPos2;
    std::string codeForPos3;
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
            // TOKEN PROCESSING - COL 7
            if (tokenCount == 7)  {
                selectedPlaylistPosition = token;
                if (token == "1") {
                    codeForPos1 = selectedRatingCode;
                    if (Constants::verbose == true)std::cout << "selectedPlaylistPosition 1 is: "<< str << std::endl;
                }
                if (token == "2") {
                    codeForPos2 = selectedRatingCode;
                    if (Constants::verbose == true)std::cout << "selectedPlaylistPosition 2 is: "<< str << std::endl;
                }
                if (token == "3") { // this is used as needed to replace rating code 1
                    codeForPos3 = selectedRatingCode;
                    if (Constants::verbose == true)std::cout << "selectedPlaylistPosition 3 is: "<< str << std::endl;
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
    std::cout << "RatioTime3 is: " << ratioTime3 << " versus std: " << s_ratingRatio3 << std::endl;
    std::cout << "RatioTime4 is: " << ratioTime4 << " versus std: " << s_ratingRatio4<< std::endl;
    std::cout << "RatioTime5 is: " << ratioTime5 << " versus std: " << s_ratingRatio5<< std::endl;
    std::cout << "RatioTime6 is: " << ratioTime6 << " versus std: " << s_ratingRatio6<< std::endl;
    std::cout << "RatioTime7 is: " << ratioTime7 << " versus std: " << s_ratingRatio7<< std::endl;
    std::cout << "RatioTime8 is: " << ratioTime8 << " versus std: " << s_ratingRatio8<< std::endl;
    }
    //Compare the ratio for each rating code on the playlist to the rating code standards set by the program.
    //variables:
    double varianceRatioTime3 = (s_ratingRatio3 - ratioTime3) / s_ratingRatio3;
    double varianceRatioTime4 = (s_ratingRatio4 - ratioTime4) / s_ratingRatio4;
    double varianceRatioTime5 = (s_ratingRatio5 - ratioTime5) / s_ratingRatio5;
    double varianceRatioTime6 = (s_ratingRatio6 - ratioTime6) / s_ratingRatio6;
    double varianceRatioTime7 = (s_ratingRatio7 - ratioTime7) / s_ratingRatio7;
    double varianceRatioTime8 = (s_ratingRatio8 - ratioTime8) / s_ratingRatio8;
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
    // Determine whether a code of 1 was added in either of the last two tracks
    // If either has a rating code of 1, reassign other rating code using codeForPos3, as applicable
    if ((codeForPos1 == "1") || (codeForPos2 == "1")) {
        if (codeForPos1 == "1"){
            codeForPos1 = codeForPos2;
            codeForPos2 = codeForPos3;
        }
        if (codeForPos2 == "1"){
            codeForPos2 = codeForPos3;
        }
    }
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

std::string selectTrack(int &s_ratingNextTrack, std::string *s_selectedTrackPath, bool &s_includeAlbumVariety){
    if (Constants::verbose == true) std::cout << "Starting selectTrack function. Rating for next track is " << s_ratingNextTrack << std::endl;
    std::fstream filestrinterval;
    filestrinterval.open ("ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "selectTrack: Error opening ratedabbr2.txt file." << std::endl;}
    std::string ratedlibrary = "ratedabbr2.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()) {
        std::cout << "selectTrack: Error opening ratedabbr2.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for ratedabbr2.txt
    std::string str2; // store the string for artistexcludes.txt
    std::string str3; // store the string for finalids.txt
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
    std::string albumID;
    static bool s_excludeMatch{false};
    static bool s_excludeMatch2{false};
    std::vector<std::string>finaltracksvect; // New vector to store final selections
    if (Constants::verbose == true) std::cout << "selectTrack function: Created new vector to store final selections" << std::endl;
    // Outer loop: iterate through ratedSongsTable in the file "ratedabbr2.txt"    
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
            if (tokenCount == 6) {albumID = token;} //just added
            // TOKEN PROCESSING - COL 7
            if (tokenCount == 7)  {playlistPos = token;}
            ++ tokenCount;
        }
        if (playlistPos == "0") {notInPlaylist = 1;} // Set variable to check whether item is or is not in the playlist
        else {notInPlaylist = 0;} // Set variable to check whether item is or is not in the playlist
        if (notInPlaylist == 0) {continue;} // If item is already on the playlist, continue to next str1
        else if (ratingCode != std::to_string(s_ratingNextTrack)) {continue;} // If item does not have the rating selected, continue to next str1
        /*
         If str1 has not yet been skipped, a track has been found with the rating selected and is not yet been placed on the playlist
         Now, open an inner loop and iterate through artistexcludes.txt, comparing each 'exclude' entry against the artist token.
         Continue to next str1 if a match found (meaning it identifies an excluded artist).
        */
        std::ifstream artistexcludes;  // Next ensure artistexcludes.txt is ready to open
        artistexcludes.open ("artistexcludes.txt");
        if (artistexcludes.is_open()) {artistexcludes.close();}
        else {std::cout << "selectTrack: Error opening artistexcludes.txt file." << std::endl;}
        std::string artistexcludes2 = "artistexcludes.txt"; // now we can use it as input file
        std::ifstream artexcludes(artistexcludes2); // Open artistexcludes.txt as ifstream
        if (!artexcludes.is_open()) {
            std::cout << "selectTrack: Error opening artistexcludes.txt." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        s_excludeMatch = false; // set default to not exclude based on artist
        while (std::getline(artexcludes, str2)) {
            if (std::string(str2) == selectedArtistToken) {s_excludeMatch = true;} // If excluded artist found, set bool to true
        }
        if (s_excludeMatch == true){
            artexcludes.close();
            continue;} // if an excluded artist is found continue to next str1
        artexcludes.close();
        /*
           If not yet skipped, and if the user has enabled the album variety feature, open another inner loop and iterate through
           finalids.txt (which contains the album IDs which are to be excluded) and compare each ID to the str1 albumID token.
           Continue to next str1 if a match found (meaning it identifies an excluded album ID).
        */
        if (s_includeAlbumVariety == true){
            std::ifstream artistalbexcludes;  // Next ensure artistalbexcludes.txt is ready to open
            artistalbexcludes.open ("finalids.txt");
            if (artistalbexcludes.is_open()) {artistalbexcludes.close();}
            else {std::cout << "selectTrack: Error opening finalids.txt file." << std::endl;}
            std::string artistalbexcludes2 = "finalids.txt"; // now we can use it as input file
            std::ifstream artalbexcludes(artistalbexcludes2); // Open artistexcludes.txt as ifstream
            if (!artalbexcludes.is_open()) {
                std::cout << "selectTrack: Error opening finalids.txt." << std::endl;
                std::exit(EXIT_FAILURE);
            }
            s_excludeMatch2 = false;
            while (std::getline(artalbexcludes, str3)) {
                trim_cruft(str3);
                trim_cruft(albumID);
                if (std::string(str3) == albumID) {
                    s_excludeMatch2 = true; // If excluded album found, set bool to true                    
                }                
            }
            if (s_excludeMatch2 == true){
                artalbexcludes.close();
                continue;}// if an excluded artist is found continue to next str1
            artalbexcludes.close();
        }
        finaltracksvect.push_back(tokenLTP+","+songPath); // If not skipped by now, add the track to the final list
        continue; // end of the str1 while block, continue to next str1
    }
    ratedSongsTable.close();
    std::sort (finaltracksvect.begin(), finaltracksvect.end()); // sorts vector by LTP so the oldest track is first
    std::string fullstring = finaltracksvect.front(); // Saves the first item in vector to a variable
    std::vector<std::string> splittedStrings = split(fullstring, ','); // Function splits the variable and leaves the track path only
    *s_selectedTrackPath = splittedStrings[1];
    if (Constants::verbose == true) std::cout << "selectTrack function: Write/append s_selectedTrackPath to the cleanedplaylist.txt file." << std::endl;    
    std::ofstream playlist(Constants::cleanedPlaylist,std::ios::app); //Write/append s_selectedTrackPath to the cleanedplaylist.txt file.
    playlist << *s_selectedTrackPath << "\n";
    playlist.close();    
    std::string selectedTrackPathshort;    
    finaltracksvect.shrink_to_fit();
    return *s_selectedTrackPath;
}

// Function to populate four variables used to determine rating code 1 track selection in function getNewTrack
void code1stats(int *_suniqueCode1ArtistCount, int *_scode1PlaylistCount, int *_slowestCode1Pos, std::string *_sartistLastCode1){
    std::vector<std::string>code1artistsvect;
    std::string selectedArtistToken; // Artist variable
    std::string str; // store the string for ratedabbr2.txt
    std::string ratingCode;
    std::string playlistPos;
    int posint{99999};
    std::fstream filestrinterval;
    filestrinterval.open ("ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "code1stats: Error opening ratedabbr2.txt file." << std::endl;}
    std::string ratedlibrary = "ratedabbr2.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()) {
        std::cout << "code1stats: Error opening ratedabbr2.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    while (std::getline(ratedSongsTable, str)) {  // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        while (std::getline(iss, token, ',')) {
            // TOKEN PROCESSING - COL 1
            if (tokenCount == 1) {ratingCode = token;}// store rating variable
            // TOKEN PROCESSING - COL 2
            if (tokenCount == 2) {selectedArtistToken = token;} //Selected artist token
            // TOKEN PROCESSING - COL 7
            if (tokenCount == 7)  {
                playlistPos = token;
                if ((ratingCode == "1") && (playlistPos != "0")){
                    ++*_scode1PlaylistCount;
                }
            }
            ++ tokenCount;
        }
        if ((ratingCode == "1") && (playlistPos != "0")){//rating code 1 track is in the playlist
            posint = std::stoi(playlistPos);
            if (*_slowestCode1Pos > posint){
                *_slowestCode1Pos = posint;
                *_sartistLastCode1 = selectedArtistToken;
            }
        }
        if ((std::find(code1artistsvect.begin(), code1artistsvect.end(), selectedArtistToken) == code1artistsvect.end()) && (ratingCode == "1")) {
            code1artistsvect.push_back(selectedArtistToken);
            ++*_suniqueCode1ArtistCount;
        }
    }
}
// Function used to select a rating code 1 track

void getNewTrack(std::string &s_artistLastCode1, std::string *s_selectedCode1Path){
    std::string returntrack;
    std::fstream filestrinterval;
    filestrinterval.open ("ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "getNewTrack: Error opening ratedabbr2.txt file." << std::endl;}
    std::string ratedlibrary = "ratedabbr2.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()) {
        std::cout << "getNewTrack: Error opening ratedabbr2.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for ratedabbr2.txt
    std::string tokenLTP;
    std::string ratingCode;
    std::string selectedArtistToken; // Artist variable
    std::string songPath;
    std::string playlistPos;
    std::string albumID;
    std::vector<std::string>code1tracksvect; // New vector to store all code 1 selections
    // Outer loop: iterate through ratedSongsTable in the file "ratedabbr2.txt"
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
            // TOKEN PROCESSING - COL 6
            if (tokenCount == 6) {albumID = token;}// store album ID variable
            // TOKEN PROCESSING - COL 7
            if (tokenCount == 7)  {playlistPos = token;}
            ++ tokenCount;
        }
        if ((ratingCode == "1") && (playlistPos == "0") &&(selectedArtistToken != s_artistLastCode1))  // if a code 1 track is not
            //in the playlist and not the last artist selected, add to vector used to return track path to s_selectedCode1Path
        {code1tracksvect.push_back(tokenLTP+","+selectedArtistToken+","+songPath+","+playlistPos);}
    }
    std::sort (code1tracksvect.begin(), code1tracksvect.end());
    std::string fullstring = code1tracksvect.front();
    std::vector<std::string> splittedStrings = split(fullstring, ',');
    returntrack = splittedStrings[2];
    *s_selectedCode1Path = returntrack;
    //Write/append s_selectedTrackPath to the cleanedplaylist.txt file.
    std::ofstream playlist(Constants::cleanedPlaylist,std::ios::app);
    playlist << *s_selectedCode1Path << "\n";
    playlist.close();
}

/*
This function runs after the function ratingCodeSelected, if enabled by the user.
Iterate through tracks in ratedabbr2.txt starting from oldest playlist position to newest
for each artist in the artistalbmexcls.txt file. Sends the IDs to a text file excludeids.txt
*/

void getAlbumIDs(){
    //if (Constants::verbose == true) std::cout << "Starting selectTrack function. Rating for next track is " << s_ratingNextTrack << std::endl;
    std::fstream filestrartists;
    filestrartists.open ("selalbmexcl.txt");
    if (filestrartists.is_open()) {filestrartists.close();}
    else {std::cout << "Error opening selalbmexcl.txt file." << std::endl;}
    std::string artistalbmexcls1 = "selalbmexcl.txt";
    std::ifstream artistTable1(artistalbmexcls1);
    if (!artistTable1.is_open()) {
        std::cout << "getArtCompare: Error opening selalbmexcl.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream trimmedlist("finalids.txt");
    std::string str1;
    std::string tokenLTP;
    std::string selectedArtistToken;
    std::string ratingCode;
    std::string playlistPos;
    std::string albumID;
    while (std::getline(artistTable1, str1)) {
        std::string str2;
        //search filestrartists2 for matching string
        std::fstream filestrartists2;
        std::fstream filestrinterval;
        filestrinterval.open ("ratedabbr2.txt");
        if (filestrinterval.is_open()) {filestrinterval.close();}
        else {std::cout << "Error opening ratedabbr2.txt file." << std::endl;}
        std::string ratedlibrary = "ratedabbr2.txt";
        std::ifstream ratedSongsTable(ratedlibrary);
        if (!ratedSongsTable.is_open()) {
            std::cout << "selectTrack: Error opening ratedabbr2.txt." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        // set variables used to compare element values in ratedabbr2 against this str1
        int finalplaylistPos = 99999;
        int finaltokenLTP = 0;
        std::string finalAlbumID = "0";
        while (std::getline(ratedSongsTable, str2)) {  // Declare variables applicable to all rows
            std::istringstream iss(str2); // str is the string of each row
            std::string token; // token is the contents of each column of data
            int tokenCount{0}; //token count is the number of delimiter characters within str
            while (std::getline(iss, token, ',')) {
                // TOKEN PROCESSING - COL 0
                if ((tokenCount == 0) && (token != "0")) {tokenLTP = token;}// get LastPlayedDate in SQL Time
                // TOKEN PROCESSING - COL 1
                if (tokenCount == 1) {ratingCode = token;}// store rating variable
                // TOKEN PROCESSING - COL 2
                if (tokenCount == 2) {selectedArtistToken = token;} //Selected artist token
                // TOKEN PROCESSING - COL 6
                if (tokenCount == 6) {albumID = token;} // store album ID variable
                // TOKEN PROCESSING - COL 7
                if (tokenCount == 7)  {
                    if (token != "0"){
                        int tmpint = std::stoi(token);
                        if (finalplaylistPos > tmpint){playlistPos = token;}
                        else playlistPos = "0";
                    }
                    else playlistPos = "0";
                }
                ++ tokenCount;
            }
            bool foundmatch{0};
            foundmatch = stringMatch(selectedArtistToken, str1);// Check whether the artist in ratedabbr2 matches artist in selalbmexcl.txt
            if ((playlistPos != "0") && (foundmatch == 1)){// If the artist matches, get lowest playlist position and save albumID associated with it
                // if the matched artist is in the playlist or extended count, get lowest playlist position
                tokenLTP = "0"; // reset lasttime played when/if playlist entry found
                int tmppos= std::stoi(playlistPos);
                finalplaylistPos = tmppos;
                finalAlbumID = albumID;
            }
            // If the artist matches, but no playlist position, get most recent lastplayed date and save albumID associated with it
            if ((playlistPos == "0") && (foundmatch == 1)){
                // if the matched artist is not in the playlist or extended count, check last time played if playlist position is zero
                int tmpltp = std::stoi(tokenLTP);
                if ((finaltokenLTP < tmpltp) && (finalplaylistPos == 99999)) {
                    finaltokenLTP = tmpltp; // update finalLTP if greater than existing finalLTP
                    finalAlbumID = albumID; // save albumID only if not already found in playlist
                }
            }
        }
        ratedSongsTable.close();
        trimmedlist << finalAlbumID << std::endl;
    }
    artistTable1.close();
    trimmedlist.close();
}

/*
At time of track selection (if user selected album variety), use the file artistalbmexcls.txt
and artistexcludes.txt to create a list of artists subject to album variety screen but excluding
those already on the excluded artists list. Output to a temp file selalbmexcl.txt
*/
void getTrimArtAlbmList(){
    std::fstream filestrartists;
    filestrartists.open ("artistalbmexcls.txt");
    if (filestrartists.is_open()) {filestrartists.close();}
    else {std::cout << "Error opening artistalbmexcls.txt file." << std::endl;}
    std::string artistalbmexcls1 = "artistalbmexcls.txt"; // now we can use it as input file
    std::ifstream artistTable1(artistalbmexcls1);
    if (!artistTable1.is_open()) {
        std::cout << "getArtCompare: Error opening artistalbmexcls.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream trimmedlist("selalbmexcl.txt");
    std::string str1;
    while (std::getline(artistTable1, str1)) {
        //search filestrartists2 for matching string
        std::fstream filestrartists2;
        filestrartists2.open ("artistexcludes.txt");
        if (filestrartists2.is_open()) {filestrartists2.close();}
        else {std::cout << "Error opening artistexcludes.txt file." << std::endl;}
        std::string artistexcludes2 = "artistexcludes.txt"; // now we can use it as input file
        std::ifstream artistTable2(artistexcludes2);
        if (!artistTable2.is_open()) {
            std::cout << "getArtCompare: Error opening artistexcludes.txt." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        bool foundmatch{0};
        foundmatch = matchLineinIfstream(artistTable2, str1);
        artistTable2.close();
        if (foundmatch == 0)
            trimmedlist << str1 << std::endl;
    }
    trimmedlist.close();
    artistTable1.close();
}
