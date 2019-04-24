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
const char kUserFileName[16]{"archsimian.conf"};  // user configuration and preferences filename
const char kSqlFileName[19]{"/exportMMTable.sql"}; //SQL file to write so that the MM.DB Songs table can be exported
const char kCleanLibFile[13]{"cleanlib.dsv"}; // filename for storing the ArchSimian basic database
const char kCleanedPlaylist[20]("cleanedplaylist.txt"); //filename for storing the current playlist

// Constant used to set the maximum size of a playlist

//const int finalplaylistPos{99999};

// Constants to set user-configurable default settings
const int kUserDefaultRepeatFreqCode1{20};
const int kUserDefaultTracksToAdd{10};
const QString kUserDefaultDefaultPlaylist{};
const bool kUserDefaultIncludeNewTracks{false};
const bool kUserDefaultIncludeAlbumVariety{false};
const bool kUserDefaultNoAutoSave{true};
const bool kUserDefaultDisableNotificationAddTracks{false};
const double kUserDefaultDaysTillRepeatCode3{65};
const double kUserDefaultRepeatFactorCode4{2.7};
const double kUserDefaultRepeatFactorCode5{2.1};
const double kUserDefaultRepeatFactorCode6{2.2};
const double kUserDefaultRepeatFactorCode7{1.6};
const double kUserDefaultRepeatFactorCode8{1.4};
const QString kUserDefaultWindowsDriveLetter{};
const int kUserDefaultMinalbums{2};
const int kUserDefaultMintrackseach{4};
const int kUserDefaultMintracks{8};

// Calendar and time-related constants
const int kDaysInYear{365};
const int kMonthsInYear{12};
const int kWeeksInYear{52};
const long kMilSecsToMinsFactor{60000};
const int kMinsToHoursFactor{60};
const int kConvertDecimalToPercentDisplay{100};
const int kDaysPerListeningPeriod{10};


// Constant to turn on or off troubleshooting feedback in console
const bool kVerbose{true};

}
#endif // CONSTANTS_H
