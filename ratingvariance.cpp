#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdio.h>

// One must be very careful when comparing floating point numbers for equality
// so instead, use this function to get 'close enough' to a match:

inline bool isEqual(double x, double y)
{
  const double epsilon =  1e-5;
  return std::abs(x - y) <= epsilon * std::abs(x);
  // see http://www.cs.technion.ac.il/users/yechiel/c++-faq/floating-point-arith.html
}

int ratingCodeSelected(double *_sratingRatio3, double *_sratingRatio4, double *_sratingRatio5, double *_sratingRatio6, double *_sratingRatio7, double *_sratingRatio8)
{
    //Lookup the rating codes for last two tracks on the playlist;
    //variables:
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

    // Iterate through ratedlib.dsv and increment the time for each rating code and total for playlist
    // COL 12 - SongLength
    // COL 18 - PlaylistPosition
    // COL 29 - RatingCode
    //variables:
    std::string str; // store the string for ratedlib.dsv
    double selectedSongLength{0.0};
    std::string selectedPlaylistPosition;
    std::string selectedRatingCode;

    std::fstream filestrinterval;
    filestrinterval.open ("ratedlib.dsv");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "Error opening ratedlib.dsv file." << std::endl;}
    std::string ratedlibrary = "ratedlib.dsv"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);    }

    while (std::getline(ratedSongsTable, str))
    {   // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str

        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, '^'))
        {
            // TOKEN PROCESSING - COL 12
            if (tokenCount == 12)  {selectedSongLength = std::atof(token.c_str());
            }
            // TOKEN PROCESSING - COL 18
            if (tokenCount == 18)  {
                selectedPlaylistPosition = token;
            }
            // TOKEN PROCESSING - COL 29
            if (tokenCount == 29)  {selectedRatingCode = token;
                if (selectedPlaylistPosition == "1") {codeForPos1 = selectedRatingCode;}
                if (selectedPlaylistPosition == "2") {codeForPos2 = selectedRatingCode;}
            }
            ++ tokenCount;
        }
        if (selectedPlaylistPosition != "0")
        {
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
    std::cout << "totalPLTime3 is: " <<totalPLTime3/1000 << std::endl;
    std::cout << "totalPLTime4 is: " <<totalPLTime4/1000 << std::endl;
    std::cout << "totalPLTime5 is: " <<totalPLTime5/1000 << std::endl;
    std::cout << "totalPLTime6 is: " <<totalPLTime6/1000 << std::endl;
    std::cout << "totalPLTime7 is: " <<totalPLTime7/1000 << std::endl;
    std::cout << "totalPLTime8 is: " <<totalPLTime8/1000 << std::endl;
    std::cout << "totalPlaylistTime is: " <<totalPlaylistTime/1000 << std::endl;

    //Calculate time ratio for each rating code by dividing each by the total playlist time.
    //variables:
    double ratioTime3 = totalPLTime3 / totalPlaylistTime;
    double ratioTime4 = totalPLTime4 / totalPlaylistTime;
    double ratioTime5 = totalPLTime5 / totalPlaylistTime;
    double ratioTime6 = totalPLTime6 / totalPlaylistTime;
    double ratioTime7 = totalPLTime7 / totalPlaylistTime;
    double ratioTime8 = totalPLTime8 / totalPlaylistTime;

    std::cout << "RatioTime3 is: " << ratioTime3 << " versus std: " << *_sratingRatio3 << std::endl;
    std::cout << "RatioTime4 is: " << ratioTime4 << " versus std: " << *_sratingRatio4<< std::endl;
    std::cout << "RatioTime5 is: " << ratioTime5 << " versus std: " << *_sratingRatio5<< std::endl;
    std::cout << "RatioTime6 is: " << ratioTime6 << " versus std: " << *_sratingRatio6<< std::endl;
    std::cout << "RatioTime7 is: " << ratioTime7 << " versus std: " << *_sratingRatio7<< std::endl;
    std::cout << "RatioTime8 is: " << ratioTime8 << " versus std: " << *_sratingRatio8<< std::endl;

    //Compare the ratio for each rating code on the playlist to the rating code standards set by the program.
    //variables:
    double varianceRatioTime3 = (*_sratingRatio3 - ratioTime3) / *_sratingRatio3;
    double varianceRatioTime4 = (*_sratingRatio4 - ratioTime4) / *_sratingRatio4;
    double varianceRatioTime5 = (*_sratingRatio5 - ratioTime5) / *_sratingRatio5;
    double varianceRatioTime6 = (*_sratingRatio6 - ratioTime6) / *_sratingRatio6;
    double varianceRatioTime7 = (*_sratingRatio7 - ratioTime7) / *_sratingRatio7;
    double varianceRatioTime8 = (*_sratingRatio8 - ratioTime8) / *_sratingRatio8;

    std::cout << "varianceRatioTime3 is: " << varianceRatioTime3 << std::endl;
    std::cout << "varianceRatioTime4 is: " << varianceRatioTime4 << std::endl;
    std::cout << "varianceRatioTime5 is: " << varianceRatioTime5 << std::endl;
    std::cout << "varianceRatioTime6 is: " << varianceRatioTime6 << std::endl;
    std::cout << "varianceRatioTime7 is: " << varianceRatioTime7 << std::endl;
    std::cout << "varianceRatioTime8 is: " << varianceRatioTime8 << std::endl;

    std::cout << "Rating for last track added was: " <<codeForPos1;
    std::cout << ", and second-to-last was: " <<codeForPos2 << std::endl;

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
    else exclude7and8 = false;

    // Condition 1
    // If both of the last two tracks was a code 7 or 8 (should not occur), exclude from consideration
    if (((codeForPos1=="7") || (codeForPos1=="8")) && ((codeForPos2 == "7") || (codeForPos2 =="8"))) {
        double a_variances[] = {vrt3, vrt4, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 4);
        std::cout << "Condition 1: ";
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
        std::cout << "Condition 2: ";
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
    }
    // Condition 3
    // If the second to last track was a code 7 or 8, and last track was a 4, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  &&  (codeForPos1=="4"))  {
        double a_variances[] = {vrt3, vrt5, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        std::cout << "Condition 3: ";
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime6)== 1) {x = 6;}
    }
    // Condition 4
    // If the second to last track was a code 7 or 8, and last track was a 5, exclude from consideration
    if (((codeForPos2 == "7") || (codeForPos2 =="8"))  && (codeForPos1=="5"))  {
        double a_variances[] = {vrt3, vrt4, vrt6};
        double* maxVariance;
        maxVariance = std::max_element(a_variances, a_variances + 3);
        std::cout << "Condition 4: ";
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
        std::cout << "Condition 5: ";
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
        std::cout << "Condition 6: ";
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
        std::cout << "Condition 7: ";
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
        std::cout << "Condition 8: ";
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
        std::cout << "Condition 9: ";
        if (isEqual(*maxVariance,varianceRatioTime3)== 1) {x = 3;}
        if (isEqual(*maxVariance,varianceRatioTime4)== 1) {x = 4;}
        if (isEqual(*maxVariance,varianceRatioTime5)== 1) {x = 5;}
        if (isEqual(*maxVariance,varianceRatioTime7)== 1) {x = 7;}
        if (isEqual(*maxVariance,varianceRatioTime8)== 1) {x = 8;}
    }
    ratedSongsTable.close();
    return x;
}
