#ifndef LASTPLAYEDDAYS_H
#define LASTPLAYEDDAYS_H
// Function to convert the time value stored by MediaMonkey and return
// the number of calendar days since the track was last played
// If the track has not yet been played, add an artificial
// "last played date" between two constants, kLowerRndBound and
// kUpperRndBound (currently 30, 120), representing # of days ago
double getLastPlayedDays (double mm4LastPlayedDays);
// Function used to generate a random lastplayed date if a rated track has not
// yet been played. The date is excluded from collection of played statistics
double getNewRandomLPDate (double x);
#endif // LASTPLAYEDDAYS_H
