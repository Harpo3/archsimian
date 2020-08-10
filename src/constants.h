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
// This will cause the program to select the unplayed tracks that have an assigned
// rating (of other than 1) and assign them to the playlist candidate list on a random basis.

// These types may be changed in a future update to allow for user-level adjustments (within a range).
const int kLowerRndBound{180};
const int kUpperRndBound{700};

// Constant used to enabled MM4 update
const bool kUserDefaultMM4Disabled{false};

// Constant used to standardize the user configuration file name for Arch
const char kUserFileName[16]{"archsimian.conf"};  // user configuration and preferences filename
const char kSqlFileName[19]{"/exportMMTable.sql"}; //SQL file to write so that the MM.DB Songs table can be exported
const char kCleanLibFile[13]{"cleanlib.dsv"}; // filename for storing the ArchSimian basic database
const char kCleanedPlaylist[20]("cleanedplaylist.txt"); //filename for storing the current playlist

// Constant for specilized application paths
const char kAIMPLogPath[46]("/Android/data/com.aimp.player/cache/debug.log");

// Constants used to set the maximum size of a playlist or max Code1 position
const int kMaxFinalPlaylistPos{99999};
const int kMaxLowestCode1Pos{99999};
const int kMaxHighestCode1Pos{99999};

// Constant for number tracks to add before notifying user of delay in processing
const int kNotifyTrackThreshold{29};

// Constants to set user-configurable default settings
const int kUserDefaultRepeatFreqCode1{20};
const int kUserDefaultTracksToAdd{10};
const QString kUserDefaultDefaultPlaylist{};
const bool kUserDefaultIncludeNewTracks{false};
const bool kUserDefaultIncludeAlbumVariety{false};
const bool kUserDefaultNoAutoSave{false};
const bool kUserDefaultDisableNotificationAddTracks{true};
const double kUserDefaultDaysTillRepeatCode3{65};
const double kUserDefaultRepeatFactorCode4{2.7};
const double kUserDefaultRepeatFactorCode5{2.1};
const double kUserDefaultRepeatFactorCode6{2.2};
const double kUserDefaultRepeatFactorCode7{1.6};
const double kUserDefaultRepeatFactorCode8{1.4};
const QString kUserDefaultWindowsDriveLetter{""};
const int kUserDefaultMinalbums{2};
const int kUserDefaultMintrackseach{4};
const int kUserDefaultMintracks{8};
const QString kWindowsTopFolder{""};
const QString kMusicLibShortened{""};
const int kPlaylistActualCntSelCode{0};

// Constants for bound limits for setting by user in ui
const int kRatingCode3MinDays{10};
const int kRatingCode3MaxDays{120};

// Calendar and time-related constants
const int kDaysInYear{365};
const int kMonthsInYear{12};
const int kWeeksInYear{52};
const int kMilSecsToMinsFactor{60000};
const int kMinsToHoursFactor{60};
const int kConvertDecimalToPercentDisplay{100};
const double kFractionOneDay{0.002739762};
const int kSecondsToMins{60};

// Constants for evaluating average listening time
const int kDaysPerListeningPeriod{10};
const double kUpperBoundPeriod1{10.9999};
const int kLowerBoundPeriod2{11};
const double kUpperBoundPeriod2{20.9999};
const int kLowerBoundPeriod3{21};
const double kUpperBoundPeriod3{30.9999};
const int kLowerBoundPeriod4{31};
const double kUpperBoundPeriod4{40.9999};
const int kLowerBoundPeriod5{41};
const double kUpperBoundPeriod5{50.9999};
const int kLowerBoundPeriod6{51};
const double kUpperBoundPeriod6{60.9999};
const double kSumOfTheYearDigitsPeriod1{0.3};
const double kSumOfTheYearDigitsPeriod2{0.25};
const double kSumOfTheYearDigitsPeriod3{0.2};
const double kSumOfTheYearDigitsPeriod4{0.15};
const double kSumOfTheYearDigitsPeriod5{0.1};
const double kSumOfTheYearDigitsPeriod6{0.05};

// Constants for identifying column number in database (DSV, CSV) files
const int kColumn0{0};
const int kColumn1{1};
const int kColumn2{2};
const int kColumn3{3};
const int kColumn4{4};
const int kColumn5{5};
const int kColumn6{6};
const int kColumn7{7};
const int kColumn8{8};
const int kColumn9{9};
const int kColumn10{10};
const int kColumn11{11};
const int kColumn12{12};
const int kColumn13{13};
const int kColumn14{14};
const int kColumn15{15};
const int kColumn16{16};
const int kColumn17{17};
const int kColumn18{18};
const int kColumn19{19};
const int kColumn20{20};
const int kColumn21{21};
const int kColumn22{22};
const int kColumn23{23};
const int kColumn24{24};
const int kColumn25{25};
const int kColumn26{26};
const int kColumn27{27};
const int kColumn28{28};
const int kColumn29{29};
const int kColumn30{30};
const int kColumn31{31};
const int kColumn32{32};
const int kColumn33{33};
const int kColumn34{34};
const int kColumn35{35};
const int kColumn36{36};

// Constants for rating codes
const int kRatingCode1{1};
const int kRatingCode3{3};
const int kRatingCode4{4};
const int kRatingCode5{5};
const int kRatingCode6{6};
const int kRatingCode7{7};
const int kRatingCode8{8};

// Constant to turn on or off troubleshooting feedback in console
const bool kVerbose{true};
}

#endif // CONSTANTS_H
