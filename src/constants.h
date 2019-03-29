#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QApplication>

namespace Constants   // Defines a namespace to hold program-level constants
{
// Two constants are used to convert the time format used by
// MediaMonkey (SQL time) to epoch time
const double kEpochConv1{25569.00};
const double kEpochConv2{86400.00};

// Two constants are used to select lower and upper bounds
// for a random number selection. The selection will be from a range
// between the lower and upper bounds representing the number of
// artificial calendar days since a never-played-track was "last played".
// This will cause the program to select the recent unrated track for the playlist
// on a random basis.
// These types may be changed in a future update to allow for user-level adjustments (within a range).
const int kLowerRndBound{30};
const int kUpperRndBound{500};

// Constant used to standardize the user configuration file name for Arch
const char userFileName[16]{"archsimian.conf"};  // user configuration and preferences filename
const char sqlFileName[19]{"/exportMMTable.sql"}; //SQL file to write so that the MM.DB Songs table can be exported
const char cleanLibFile[13]{"cleanlib.dsv"}; // filename for storing the ArchSimian basic database
const char cleanedPlaylist[20]("cleanedplaylist.txt"); //filename for storing the current playlist

// Constant to turn on or off troubleshooting feedback in console
const bool verbose{false};

}
#endif // CONSTANTS_H