#include <QDir>
#include <fstream>
#include <sstream>
#include "basiclibfunctions.h"
#include "getplaylist.h"
#include "constants.h"
#include "utilities.h"

// One must be very careful when comparing floating point numbers for equality
// so instead, use this function to get 'close enough' to a match:
inline bool isEqual(double x, double y){
    const double epsilon =  1e-5;
    return std::abs(x - y) <= epsilon * std::abs(x);
    // see http://www.cs.technion.ac.il/users/yechiel/c++-faq/floating-point-arith.html
}

int ratingCodeSelected(double &s_ratingRatio3, double &s_ratingRatio4, double &s_ratingRatio5,
                       double &s_ratingRatio6, double &s_ratingRatio7, double &s_ratingRatio8){
    //Lookup the rating codes for last two tracks on the playlist;
    if (Constants::kVerbose) std::cout << "ratingCodeSelected function started." << std::endl;
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    int x = 0; // variable to return the rating code to be used for the next track selection 
    std::string codeForPos1{"3"};
    std::string codeForPos2{"4"};
    std::string codeForPos3{"5"};
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
    filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "ratingCodeSelected: Error opening ratedabbr2.txt file." << std::endl;}
    std::string ratedlibrary = appDataPathstr.toStdString()+"/ratedabbr2.txt"; // now we can use it as input file
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
            if (tokenCount == Constants::kColumn1)  {
                selectedRatingCode = token;
            }
            if (tokenCount == Constants::kColumn4)  {selectedSongLength = std::atof(token.c_str());
            }
            if (tokenCount == Constants::kColumn7)  {
                selectedPlaylistPosition = token;
                if (token == "1") {
                    codeForPos1 = selectedRatingCode;
                    if (Constants::kVerbose)std::cout << "selectedPlaylistPosition 1 is: "<< str << std::endl;
                }
                if (token == "2") {
                    codeForPos2 = selectedRatingCode;
                    if (Constants::kVerbose)std::cout << "selectedPlaylistPosition 2 is: "<< str << std::endl;
                }
                if (token == "3") { // this is used as needed to replace rating code 1
                    codeForPos3 = selectedRatingCode;
                    if (Constants::kVerbose)std::cout << "selectedPlaylistPosition 3 is: "<< str << std::endl;
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
    if (Constants::kVerbose) {
    std::cout << "totalPLTime3 is: " <<totalPLTime3/(Constants::kMilSecsToMinsFactor/Constants::kMinsToHoursFactor) << std::endl;
    std::cout << "totalPLTime4 is: " <<totalPLTime4/(Constants::kMilSecsToMinsFactor/Constants::kMinsToHoursFactor) << std::endl;
    std::cout << "totalPLTime5 is: " <<totalPLTime5/(Constants::kMilSecsToMinsFactor/Constants::kMinsToHoursFactor) << std::endl;
    std::cout << "totalPLTime6 is: " <<totalPLTime6/(Constants::kMilSecsToMinsFactor/Constants::kMinsToHoursFactor) << std::endl;
    std::cout << "totalPLTime7 is: " <<totalPLTime7/(Constants::kMilSecsToMinsFactor/Constants::kMinsToHoursFactor) << std::endl;
    std::cout << "totalPLTime8 is: " <<totalPLTime8/(Constants::kMilSecsToMinsFactor/Constants::kMinsToHoursFactor) << std::endl;
    std::cout << "totalPlaylistTime is: " <<totalPlaylistTime/(Constants::kMilSecsToMinsFactor/Constants::kMinsToHoursFactor) << std::endl;
    }
    //Calculate time ratio for each rating code by dividing each by the total playlist time.
    //variables:
    double ratioTime3 = totalPLTime3 / totalPlaylistTime;
    double ratioTime4 = totalPLTime4 / totalPlaylistTime;
    double ratioTime5 = totalPLTime5 / totalPlaylistTime;
    double ratioTime6 = totalPLTime6 / totalPlaylistTime;
    double ratioTime7 = totalPLTime7 / totalPlaylistTime;
    double ratioTime8 = totalPLTime8 / totalPlaylistTime;
    if (Constants::kVerbose) {
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
    if (Constants::kVerbose) {
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
    exclude7and8 = (codeForPos1 == "7") || (codeForPos1 == "8") || (codeForPos2 == "7") || (codeForPos2 == "8");
    // Condition 1
    // If both of the last two tracks was a code 7 or 8 (should not occur), exclude from consideration
    if (((codeForPos1=="7") || (codeForPos1=="8")) && ((codeForPos2 == "7") || (codeForPos2 =="8"))) {
        double a_variances[] = {vrt3, vrt4, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 4);
        if (Constants::kVerbose) std::cout << "Condition 1. Evaluating codes 3, 4, 5, 6." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = Constants::kRatingCode3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = Constants::kRatingCode4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = Constants::kRatingCode5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = Constants::kRatingCode6;}
    }
    // Condition 1a
    // If the last track was either a code 7 or 8, exclude 7 and 8 from consideration
    if ((codeForPos1=="7") || (codeForPos1=="8")) {
        double a_variances[] = {vrt3, vrt4, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 4);
        if (Constants::kVerbose) std::cout << "Condition 1a. Evaluating codes 3, 4, 5, 6." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = Constants::kRatingCode3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = Constants::kRatingCode4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = Constants::kRatingCode5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = Constants::kRatingCode6;}
    }
    // Condition 2
    // If the second to last track was a code 7 or 8, and last track was a 3, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  && (codeForPos1=="3")) {
        double a_variances[] = {vrt4, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        if (Constants::kVerbose) std::cout << "Condition 2. Evaluating codes 4, 5, 6." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = Constants::kRatingCode4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = Constants::kRatingCode5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = Constants::kRatingCode6;}
    }
    // Condition 3
    // If the second to last track was a code 7 or 8, and last track was a 4, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  &&  (codeForPos1=="4")) {
        double a_variances[] = {vrt3, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        if (Constants::kVerbose) std::cout << "Condition 3. Evaluating codes 3, 5, 6." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = Constants::kRatingCode3;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = Constants::kRatingCode5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = Constants::kRatingCode6;}
    }
    // Condition 4
    // If the second to last track was a code 7 or 8, and last track was a 5, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  && (codeForPos1=="5")) {
        double a_variances[] = {vrt3, vrt4, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        if (Constants::kVerbose) std::cout << "Condition 4. Evaluating codes 3, 4, 6." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = Constants::kRatingCode3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = Constants::kRatingCode4;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = Constants::kRatingCode6;}
    }
    // Condition 5
    // If the second to last track was a code 7 or 8, and last track was a 6, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  && (codeForPos1=="6")) {
        double a_variances[] = {vrt3, vrt4, vrt5};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        if (Constants::kVerbose) std::cout << "Condition 5. Evaluating codes 3, 4, 5." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = Constants::kRatingCode3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = Constants::kRatingCode4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = Constants::kRatingCode5;}
    }
    // Condition 6
    // If neither of the last two tracks was a code 7 or 8, and last track was a 3, exclude from consideration
    if ((!exclude7and8) && (codeForPos1=="3")) {
        double a_variances[] = {vrt4, vrt5, vrt6, vrt7, vrt8};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 5);
        if (Constants::kVerbose) std::cout << "Condition 6. Evaluating codes 4, 5, 6, 7, 8." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = Constants::kRatingCode4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = Constants::kRatingCode5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = Constants::kRatingCode6;}
        if (isEqual(*maxVariance,varianceRatioTime7)== 1) {x = Constants::kRatingCode7;}
        if (isEqual(*maxVariance,varianceRatioTime8)== 1) {x = Constants::kRatingCode8;}
    }
    // Condition 7
    // If neither of the last two tracks was a code 7 or 8, and last track was a 4, exclude from consideration
    if ((!exclude7and8) && (codeForPos1=="4")) {
        double a_variances[] = {vrt3, vrt5, vrt6, vrt7, vrt8};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 5);
        if (Constants::kVerbose) std::cout << "Condition 7. Evaluating codes 3, 5, 6, 7, 8." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = Constants::kRatingCode3;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = Constants::kRatingCode5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = Constants::kRatingCode6;}
        if (isEqual(*maxVariance,varianceRatioTime7)== 1) {x = Constants::kRatingCode7;}
        if (isEqual(*maxVariance,varianceRatioTime8)== 1) {x = Constants::kRatingCode8;}
    }
    // Condition 8
    // If neither of the last two tracks was a code 7 or 8, and last track was a 5, exclude from consideration
    if ((!exclude7and8) && (codeForPos1=="5")) {
        double a_variances[] = {vrt3, vrt4, vrt6, vrt7, vrt8};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 5);
        if (Constants::kVerbose) std::cout << "Condition 8. Evaluating codes 3, 4, 6, 7, 8." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = Constants::kRatingCode3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = Constants::kRatingCode4;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = Constants::kRatingCode6;}
        if (isEqual(*maxVariance,varianceRatioTime7)== 1) {x = Constants::kRatingCode7;}
        if (isEqual(*maxVariance,varianceRatioTime8)== 1) {x = Constants::kRatingCode8;}
    }
    // Condition 9
    // If neither of the last two tracks was a code 7 or 8, and last track was a 6, exclude from consideration
    if ((!exclude7and8) && (codeForPos1=="6")) {
        double a_variances[] = {vrt3, vrt4, vrt5, vrt7, vrt8};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 5);
        if (Constants::kVerbose) std::cout << "Condition 9. Evaluating codes 3, 4, 5, 7, 8." << std::endl;
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = Constants::kRatingCode3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = Constants::kRatingCode4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = Constants::kRatingCode5;}
        if (isEqual(*maxVariance,varianceRatioTime7)== 1) {x = Constants::kRatingCode7;}
        if (isEqual(*maxVariance,varianceRatioTime8)== 1) {x = Constants::kRatingCode8;}
    }
    ratedSongsTable.close();
    return x;
}

//Screen out tracks in ratedabbr2.txt that do not match the rating of the s_ratingNextTrack variable
// Screen out tracks that have an artist that matches any when iterating through the artistexcludes.txt file
// Send remaining to a new vector, sort vector to select the oldest dated track for addition to the playlist
// Write/append the cleanedplaylist.txt file the oldest dated track found.

std::string selectTrack(int &s_ratingNextTrack, std::string *s_selectedTrackPath, bool &s_includeAlbumVariety){
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    if (Constants::kVerbose) std::cout << "Starting selectTrack function. Rating for next track is " << s_ratingNextTrack << std::endl;
    std::fstream filestrinterval;
    filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "selectTrack: Error opening ratedabbr2.txt file (514)." << std::endl;}
    std::string ratedlibrary = appDataPathstr.toStdString()+"/ratedabbr2.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()) {
        std::cout << "selectTrack: Error opening ratedabbr2.txt (518)." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for ratedabbr2.txt
    std::string str2; // store the string for artistexcludes.txt
    std::string str3; // store the string for finalids.txt
    bool notInPlaylist{false};
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
    finaltracksvect.reserve(10000);
    if (Constants::kVerbose) std::cout << "selectTrack function: Created new vector to store final selections" << std::endl;
    // Outer loop: iterate through ratedSongsTable in the file "ratedabbr2.txt"    
    while (std::getline(ratedSongsTable, str1)) {  // Declare variables applicable to all rows
        std::istringstream iss(str1); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, ',')) {
            if ((tokenCount == Constants::kColumn0) && (token != "0")) {tokenLTP = token;}// get LastPlayedDate in SQL Time
            if (tokenCount == Constants::kColumn1) {ratingCode = token;}// store rating variable
            if (tokenCount == Constants::kColumn2) {selectedArtistToken = token;} //Selected artist token
            if (tokenCount == Constants::kColumn3) {songPath = token;}// store song path variable
            if (tokenCount == Constants::kColumn4) {songLengtha = token;} //just added
            if (tokenCount == Constants::kColumn5) {artistIntervala = token;} //just added
            if (tokenCount == Constants::kColumn6) {albumID = token;} //just added
            if (tokenCount == Constants::kColumn7)  {playlistPos = token;}
            ++ tokenCount;
        }
        if (playlistPos == "0") {notInPlaylist = true;} // Set variable to check whether item is or is not in the playlist
        else {notInPlaylist = false;} // Set variable to check whether item is or is not in the playlist
        if (notInPlaylist == 0) {continue;} // If item is already on the playlist, continue to next str1
        if (ratingCode != std::to_string(s_ratingNextTrack)) {continue;} // If item does not have the rating selected, continue to next str1
        /*
         If str1 has not yet been skipped, a track has been found with the rating selected and is not yet been placed on the playlist
         Now, open an inner loop and iterate through artistexcludes.txt, comparing each 'exclude' entry against the artist token.
         Continue to next str1 if a match found (meaning it identifies an excluded artist).
        */        
        QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
        std::ifstream artistexcludes;  // Next ensure artistexcludes.txt is ready to open
        artistexcludes.open (appDataPathstr.toStdString()+"/artistexcludes.txt");
        if (artistexcludes.is_open()) {artistexcludes.close();}
        else {std::cout << "selectTrack: Error opening artistexcludes.txt file." << std::endl;}
        std::string artistexcludes2 = appDataPathstr.toStdString()+"/artistexcludes.txt"; // now we can use it as input file
        std::ifstream artexcludes(artistexcludes2); // Open artistexcludes.txt as ifstream
        if (!artexcludes.is_open()) {
            std::cout << "selectTrack: Error opening artistexcludes.txt." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        s_excludeMatch = false; // set default to not exclude based on artist
        while (std::getline(artexcludes, str2)) {
            if (std::string(str2) == selectedArtistToken) {s_excludeMatch = true;} // If excluded artist found, set bool to true
        }
        if (s_excludeMatch){
            artexcludes.close();
            continue;} // if an excluded artist is found continue to next str1
        artexcludes.close();
        /*
           If not yet skipped (!s_excludeMatch), and if the user has enabled the album variety feature, open another inner loop and iterate through
           finalids.txt (which contains the album IDs which are to be excluded) and compare each ID to the str1 albumID token.
           Continue to next str1 if a match found (meaning it identifies an excluded album ID).
        */
        if ((s_includeAlbumVariety) && (!s_excludeMatch)){ // added condition on 11 Apr 2020 ---> && (!s_excludeMatch)           
            QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
            std::ifstream artistalbexcludes;  // Next ensure artistalbexcludes.txt is ready to open
            artistalbexcludes.open (appDataPathstr.toStdString()+"/finalids.txt");
            if (artistalbexcludes.is_open()) {artistalbexcludes.close();}
            else {std::cout << "selectTrack: Error opening finalids.txt file." << std::endl;}
            std::string artistalbexcludes2 = appDataPathstr.toStdString()+"/finalids.txt"; // now we can use it as input file
            std::ifstream artalbexcludes(artistalbexcludes2); // Open artistalbexcludes.txt as ifstream
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
            if (s_excludeMatch2){
                artalbexcludes.close();
                continue;}// if an excluded artist is found continue to next str1
            artalbexcludes.close();
        } 
        finaltracksvect.push_back(tokenLTP+","+songPath); // If not skipped by now, add the track to the final list
        // end of the str1 while block, continue to next str1
    }
    ratedSongsTable.close();
    std::sort (finaltracksvect.begin(), finaltracksvect.end()); // sorts vector by LTP so the oldest track is first
    std::string fullstring = finaltracksvect.front(); // Saves the first item in vector to a variable
    std::vector<std::string> splittedStrings = split(fullstring, ','); // Function splits the variable and leaves the track path only
    *s_selectedTrackPath = splittedStrings[1];
    if (Constants::kVerbose) std::cout << "selectTrack function: Write/append s_selectedTrackPath to the cleanedplaylist.txt file." << std::endl;    
    std::ofstream playlist(appDataPathstr.toStdString()+"/cleanedplaylist.txt",std::ios::app); //Write/append s_selectedTrackPath to the cleanedplaylist.txt file.
    playlist << *s_selectedTrackPath << "\n";
    playlist.close();    
    std::string selectedTrackPathshort;
    if (Constants::kVerbose) std::cout << "selectTrack function: Track selected and added (non-code-1): " << *s_selectedTrackPath  << std::endl;
    finaltracksvect.shrink_to_fit();
    return *s_selectedTrackPath;
}
