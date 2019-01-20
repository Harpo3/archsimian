#include <QCoreApplication>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <iomanip>
#include "constants.h"
#include <random> // for std::mt19937
#include <ctime> // for std::time

// Function to convert the time value stored by MediaMonkey and return
// the number of calendar days since the track was last played
// If the track has not yet been played, add an artificial
// "last played date" between two constants, kLowerRndBound and
// kUpperRndBound (currently 30, 500), representing # of days ago
//sample code for main (2 lines)
//    float x = 43230.657113f;
//    x = getLastPlayedDays(x);
double getLastPlayedDays (double x){
    // This provides a variable (currDate) to store the current date in epoch time format
    double currDate = std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();
    // Need to test whether the track has been played before. If it has not,
    // MM4 assigns a float of 0.0, so use a conditional to test whether the
    //  number is 0.0. See constants.h for info on constants used.
    if (x > 1.00) { // If track has been played before - get last played in days
        // Two constants are used to convert the time format used by
        // MediaMonkey (SQL time) to epoch time (see formula below):
        //      const int kEpochConv1{25569}
        //      const int kEpochConv2{86400}
        // The conversion formula for epoch time to SQL time is: x = (x / 86400) + 25569,
        // so the conversion from SQL time to epoch time is: x = (x - 25569) * 86400
        // The below formula converts the SQL time value "x" (x) to epoch time,
        x = (x - Constants::kEpochConv1) * Constants::kEpochConv2;
        // Below takes the converted epoch time above and computes the number of
        // calendar days since the track was last played
        x = (currDate - x)/Constants::kEpochConv2;
        //std::cout << "Last played in calendar days: " << std::fixed
        //          << std::setprecision(0) << x << "\n";
    }
    else { // Else MM4 lastplayed value is less than 1, the track has not been played before.Generate a random lastplayed date.
        std::random_device rd;     // only used once to initialise (seed) engine
        std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
        std::uniform_int_distribution<int> uni(Constants::kLowerRndBound,Constants::kUpperRndBound); // guaranteed unbiased
        auto random_integer = uni(rng);          // returns random num of calendar days currently between 30-500
        x = currDate - (random_integer * Constants::kEpochConv2);
        std::cout << "Recent unrated track. Epoch date random selected for " << random_integer << " days ago is: " << std::fixed
                  << std::setprecision(0) << x << ".\n";
        // returns a random last played date in epoch time between
        // 30-500 days before the current date
        //Later, add another if statement in this else section. It will set unplayed tracks to a lastplayed date
        // of currDate (today), thereby excluding unplayed tracks from playlistincludsion.
        // User selection (create bool for include recent unrated tracks or not) will determine whether recent unrated tracks are included.
    }
    return x;    // Return the variable's value generated from either the "if" or the "else" statement
}
// Function used to generate a random lastplayed date if a rated track has not
// yet been played. The date is excluded from collection of played statistics.
// This provides a variable to store the random date in SQL time format

double getNewRandomLPDate (double x)
{
    double currDate = std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();
    // Need to test whether the track has been played before. If it has not,
    // MM4 assigns a float of 0.0, so use a conditional to test whether the
    //  number is 0.0. See constants.h for info on constants used.
    if (x < 1.00) { //  MM4 lastplayed value is less than 1, the track has not been played before.
        std::random_device rd;  // Generate a random lastplayed date.only used once to initialise (seed) engine
        std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
        std::uniform_int_distribution<int> uni(Constants::kLowerRndBound,Constants::kUpperRndBound); // guaranteed unbiased
        auto random_integer = uni(rng);          // returns random num of calendar days currently between 30-500
        // The conversion formula for epoch time to SQL time is: x = (x / 86400) + 25569
        x = ((currDate - (random_integer * Constants::kEpochConv2)) / Constants::kEpochConv2) + Constants::kEpochConv1;
        //std::cout << "Recent unrated track. Epoch date random selected for " << random_integer << " days ago is: " << std::fixed
        // << std::setprecision(0) << x << ".\n"; // returns a random last played date in epoch time between
        // 30-120 days before the current date}
    }
    return x;    // Return the variable's value generated from either the "if" or the "else" statement
}
//Later, add another if statement in this else section. It will set unplayed tracks to a lastplayed date
// of currDate (today), thereby excluding unplayed tracks from playlist inclusion.
// User selection (create bool for include recent unrated tracks or not) will determine whether recent unrated tracks are included.
