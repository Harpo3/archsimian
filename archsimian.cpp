//archsimian.cpp is equivalent to MainWindow.cpp. This and archsimian.h are
// where the UI code resides. The main () function is located at main.cpp

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "archsimian.h"
#include "constants.h"
#include "ui_archsimian.h"
#include "userconfig.h"
#include "dependents.h"
//#include "getcleanlib.h"
//#include "getlibvariables.h"
//#include "getrated.h"
//#include "getcleanlib.h"
#include "getplaylist.h"
//#include "getartistadjustedount.h"
//#include "addintervalvalues.h"
#include "writesqlfile.h"
//#include "ratingvariance.h"
//#include "getsubset.h"
//#include "selecttrack.h"

#include "basiclibfunctions.h"
#include "playlistfunctions.h"


template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N]) {//Function to execute command line with parameters
    assert((N > 0) && (argv[N - 1] == nullptr));
    return execvp(file, const_cast<char* const*>(argv));
}

inline bool doesFileExist (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

// VARIABLE DECLARATIONS & INITIALIZATIONS
static bool s_bool1{false};
static bool s_bool2{false};
static bool s_bool3{false};
static bool s_bool4{false};
static bool s_bool5{false};
static bool s_bool6{false};
static bool s_bool7{false};
static bool s_bool8{false};
static bool s_bool9{false};
static bool s_bool10{false};
static bool s_bool11{false};

const std::string cleanLibFile("cleanlib.dsv");
const std::string cleanedPlaylist("cleanedplaylist.txt");
static std::string s_mmbackupdbdirname{""};
static std::string s_musiclibrarydirname{""};
static std::string s_mmbackuppldirname{""};
static std::string s_selectedplaylist{""};
// Repeat factor codes used to calculate repeat rate in years
static double s_SequentialTrackLimit = 0;
static double s_daysTillRepeatCode3 = 65.0;
static double s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
static double s_repeatFactorCode4 = 2.7;
static double s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
static double s_repeatFactorCode5 = 2.1;
static double s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
static double s_repeatFactorCode6 = 2.2;
static double s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
static double s_repeatFactorCode7 = 1.6;
static double s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
static double s_repeatFactorCode8 = 1.4;
static double s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
// Variables declared for use with artist calculations
static double s_yrsTillRepeatCode3factor = 1 / s_yrsTillRepeatCode3;
static double s_yrsTillRepeatCode4factor = 1 / s_yrsTillRepeatCode4;
static double s_yrsTillRepeatCode5factor = 1 / s_yrsTillRepeatCode5;
static double s_yrsTillRepeatCode6factor = 1 / s_yrsTillRepeatCode6;
static double s_yrsTillRepeatCode7factor = 1 / s_yrsTillRepeatCode7;
static double s_yrsTillRepeatCode8factor = 1 / s_yrsTillRepeatCode8;
// Variables declared to calculate rating ratios
static double s_ratingRatio3{0.0},s_ratingRatio4{0.0},s_ratingRatio5{0.0},
s_ratingRatio6{0.0},s_ratingRatio7{0.0},s_ratingRatio8{0.0};
// Declare statistical variables to be collected
// First group is declared to get times and track quantities by rating group
static int s_rCode0TotTrackQty{0}, s_rCode0MsTotTime{0},
s_rCode1TotTrackQty{0}, s_rCode1MsTotTime{0},s_rCode3TotTrackQty{0},s_rCode3MsTotTime{0},
s_rCode4TotTrackQty{0}, s_rCode4MsTotTime{0},s_rCode5TotTrackQty{0},s_rCode5MsTotTime{0},
s_rCode6TotTrackQty{0},s_rCode6MsTotTime{0},s_rCode7TotTrackQty{0},s_rCode7MsTotTime{0},
s_rCode8TotTrackQty{0},s_rCode8MsTotTime{0};
// Second group is declared to get recently played statistics for last six 10-day periods
// First range is between yesterday and eleven days ago, so first count is
// all values between (currSQLDate - 1) and (currSQLDate - 10.999)
// then (currSQLDate - 11) and (currSQLDate - 20.999) and so on to 60.999
// so anything greater than currSQLDate - 60.999 is counted, then subcounted to each 10 day period
static int s_SQL10TotTimeListened{0},s_SQL10DayTracksTot{0},s_SQL20DayTracksTot{0},
s_SQL30DayTracksTot{0},s_SQL40DayTracksTot{0},s_SQL50DayTracksTot{0},s_SQL60DayTracksTot{0};
static double s_SQL20TotTimeListened{0},s_SQL30TotTimeListened{0},s_SQL40TotTimeListened{0},
s_SQL50TotTimeListened{0},s_SQL60TotTimeListened{0};
static double s_listeningRate{0.0};
static int s_totalRatedQty{0};
static double s_totalRatedTime{0.0};
static int s_totalLibQty{0};
static double s_DaysBeforeRepeatCode3{0.0};
static double s_totHrsLast60Days{0.0};
static double s_totalAdjRatedQty{0.0};
static int s_playlistSize{0};
static int s_ratingNextTrack{0};
static std::string s_MMdbDate{""};
static std::string s_LastTableDate{""};
static long s_histCount{0};

ArchSimian::ArchSimian(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ArchSimian)
{
    //
    //*********************************************************************
    // CHECK WHETHER USER CONFIGURATION EXISTS, IF NOT, USE NEW SETUP OPTIONS

    // 1. Determine if user configuration exists:  Run isConfigSetup() function (bool1) - function ran in main window
    //     if configuration file has no data in it, set bool1 to false. File is archsimian.conf

    //*********************************************************************
    //
    // User configuration: set default state to "false" for user config reset buttons
    ui->setupUi(this);
    ui->setlibraryButtonReset->setVisible(false);
    ui->setmmplButtonReset->setVisible(false);
    ui->setmmdbButtonReset->setVisible(false);

    //Check whether the configuration file currently has any data in it
    std::streampos archsimianconfsize;
    char * memblock;
    std:: ifstream file ("archsimian.conf", std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
        archsimianconfsize = file.tellg();
        memblock = new char [archsimianconfsize];
        file.seekg (0, std::ios::beg);
        file.read (memblock, archsimianconfsize);
        file.close();
        delete[] memblock;
    }
    else std::cout << "Step 1. Unable to open archsimian.conf configuration file";

    //If configuration has already been set, populate the ui labels accordingly
    if (archsimianconfsize != 0)
    {
        std::cout << "Step 1. Configuration has already been set. s_bool1 = true."<<std::endl;
        s_bool1 = true;
        //ui->setCurrentIndex(0);
        // getConfigEntry: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
        std::string s_musiclibrarydirname = userconfig::getConfigEntry(1);
        ui->setlibrarylabel->setText(QString::fromStdString(s_musiclibrarydirname));
        //dim the setlibraryButton button
        ui->setlibraryButton->setEnabled(false);
        //enable the reset button
        ui->setlibraryButtonReset->setVisible(true);
        s_mmbackuppldirname = userconfig::getConfigEntry(3);
        ui->setmmpllabel->setText(QString::fromStdString(s_mmbackuppldirname));
        //dim the setmmplButton button
        ui->setmmplButton->setEnabled(false);
        //enable the reset button
        ui->setmmplButtonReset->setVisible(true);
        std::string s_mmbackupdbdirname = userconfig::getConfigEntry(5);
        ui->setmmdblabel->setText(QString::fromStdString(s_mmbackupdbdirname));
        std::string selectedplaylist = userconfig::getConfigEntry(7);
        ui->setgetplaylistLabel->setText("Selected: " + QString::fromStdString(selectedplaylist));
        //dim the setmmdbButton button
        ui->setmmdbButton->setEnabled(false);
        //enable the reset button
        ui->setmmdbButtonReset->setVisible(true);

        bool needUpdate = recentlyUpdated();

        //bool needUpdate = isLibRefreshNeeded(); // function isLibRefreshNeeded() is from dependents.cpp
        std::cout << "Step 1. Checking isLibRefreshNeeded(): "<<needUpdate<<std::endl;
        if (needUpdate == 0)
        {
            s_MMdbDate = getMMdbDate();
            s_LastTableDate = getLastTableDate();
            ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(s_MMdbDate)+
                                           tr(", Library date: ")+ QString::fromStdString(s_LastTableDate) + "\n No update is needed.");
            // dim update library button
            ui->refreshdbButton->setEnabled(false);
        }
    }

    else {  // Otherwise, configuration has not been set. Load instructions for user to locate and set config
        // Initially, only the first of three config buttons is activated. Build the file in sequence.
        s_bool1 = false;
        ui->setlibrarylabel->setText(tr("Select the base directory of "
                                        "your music library"));
        ui->setlibraryButton->setEnabled(true);
        ui->setlibraryButtonReset->setVisible(false);
        ui->setmmpllabel->setText(tr("Select the shared Windows directory"
                                     " where you manually exported your playlist(s) from MediaMonkey"));
        ui->setmmplButton->setEnabled(false);
        ui->setmmplButtonReset->setVisible(false);
        ui->setmmdblabel->setText(tr("Select the shared Windows directory"
                                     " where you stored the MediaMonkey database backup file (MM.DB)"));
        ui->setmmdbButton->setEnabled(false);
        ui->setmmdbButtonReset->setVisible(false);
        ui->setgetplaylistLabel->setText(tr("Select playlist for adding tracks"));
    }

    //2. Determine if MM.DB database file exists: Run doesFileExist (const std::string& name) function (sets bool2).
    //    a. If bool1 is true, but bool2 is false, report to user that MM.DB was not found at the location specified and set bool1 to false
    //    b. If bool1 is false, set bool2 to false

    std::string mmdbdir = userconfig::getConfigEntry(5); // z: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath);
    const std::string mmpath = mmdbdir + "/MM.DB";
    s_bool2 = doesFileExist(mmpath);
    std::cout << "Step 2. MM.DB file exist check. s_bool2: "<< s_bool2 << std::endl;
    if (s_bool1 == false) {
        s_bool2 = false;
        s_bool4 = false;
        s_bool6 = false;
    }
    if ((s_bool1 == true) && (s_bool2 == false)) {
        std::cout << "Step 2. MM.DB was not found at the location specified. Setting s_bool1 to false." << std::endl;
        s_bool1 = false;
    }

    //3. Determine if Archsimian songs table exists: If user configuration exists and MM4 data exists (bool1 and bool2 are true),
    //determine if cleanlib.dsv songs table exists in AS, function doesFileExist (const std::string& name)  (dir paths corrected,
    // imported from MM.DB) (sets bool3)

    if ((s_bool1 == true) && (s_bool2 == true)) {
        bool tmpbool;
        tmpbool = doesFileExist(cleanLibFile);
        if (tmpbool == true){ // check that file is not empty
            //Check whether the configuration file currently has any data in it
            std::streampos cleanLibFilesize;
            char * memblock;
            std:: ifstream file (cleanLibFile, std::ios::in|std::ios::binary|std::ios::ate);
            if (file.is_open())
            {
                cleanLibFilesize = file.tellg();
                memblock = new char [cleanLibFilesize];
                file.seekg (0, std::ios::beg);
                file.read (memblock, cleanLibFilesize);
                file.close();
                delete[] memblock;
            }

            if (cleanLibFilesize != 0) {s_bool3 = true;}//doesFileExist(cleanLibFile);
        }
    }
    std::cout << "Step 3. if ((s_bool1 == true) && (s_bool2 == true)). s_bool3: "<< s_bool3 << std::endl;



    // 4. Determine if MM.DB was recently updated: If user configuration exists, MM4 data exists, and songs table cleanlib.dsv
    // exists (bool1, bool2, bool3 are all true), determine if MM4 data was recently updated by comparing MM.DB file date
    // (function getMMdbDate(), s_MmdbDate) to imported songs table file date (function getLastTableDate(), s_LastTableDate)(sets bool4)

    //*************************************
    //update using  dependents_revised file for changes to function recentlyUpdated() for dependents.cpp
    //***************************************
    std::cout << "Step 4. if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == true)). s_bool4: "<< s_bool4 << std::endl;
    if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == true)) {s_bool4 = recentlyUpdated();}

    // set ui labels if MM.DB was recently updated
    if (s_bool4 == true) // bool s_bool4: 1 means refresh DB, 0 means skip
    {
        ui->refreshdbButton->setEnabled(true);
        ui->updatestatusLabel->setText(tr("MM.DB has been recently backed up. Need to refresh data."));}

    // 5. If user configuration and MM4 data exist, but the songs table does not, import songs table into Archsimian: If user configuration
    // and MM4 data exist, but the songs table does not (bool1, bool2 are true, bool3 is false), import songs table into AS, by running
    // writeSQLFile() function, which creates the temporary basic table file libtable.dsv; then run the getLibrary() function, which creates
    // the refined table file cleanlib.dsv [NEED TO SPLIT FUNCTION getCleanLib(). ] The getLibrary() function completes the
    // following refinements: (a) corrects the directory paths to Linux, (b) adds random lastplayed dates for rated or "new-need-to-be-rated"
    // tracks that have no play history, (c) creates rating codes for any blank values found in GroupDesc col, using POPM values,
    // and (d) creates Artist codes (using col 1) and places the code in Custom2 if Custom2 is blank. Then set bool3 to true,  rechecking,
    // run doesFileExist (const std::string& name) function. After verifying  cleanlib.dsv exists, remove temporary basic table file
    // libtable.dsv Evaluates bool3 for existence of cleanlib.dsv (cleanLibFile)

    std::cout << "If bool3 = false (songs table exists), skipping step 5. Bool3:"<<s_bool3 << std::endl;

    if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == false)) {
        std::cout << "Step 5. if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == false)). s_bool1:"<< s_bool1 <<", s_bool2:"<< s_bool2 <<", s_bool3:"<< s_bool3 << std::endl;
        writeSQLFile();
        pid_t c_pid;// Create fork object; child to get database table into a dsv file, then child to open that table only
        // after it finishes getting written, not before.
        c_pid = fork(); // Run fork function
        int status; // For status of pid process
        if( c_pid == 0 ){ // Child process: Get songs table from MM4 database, and create libtable.dsv with table;
            std::string s_mmbackupdbdirname = userconfig::getConfigEntry(5); // 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
            // revise for QStandardPaths class if this does not set with makefile for this location
            const std::string sqlpathdirname = getenv("HOME");
            std::string path1 = s_mmbackupdbdirname + "/MM.DB";
            std::string path2 = ".read " + sqlpathdirname + "/exportMMTable.sql";
            const char* const argv[] = {" ", path1.c_str(), path2.c_str(), nullptr};
            execvp("sqlite3", argv);
            perror("execvp");
            if (execvp("sqlite3", argv) == -1)
                exit(EXIT_FAILURE);
        }
        else if (c_pid > 0){  // Parent process starts here. Write from libtable.dsv and gather stats
            // First, reopen libtable.dsv, clean track paths, and output to cleanlib.dsv
            //Check to ensure the file has finshed being written
            if( (c_pid = wait(&status)) < 0){
                perror("wait");
                _exit(1);
            }
            //******************************************************************
            getLibrary(); // update (add) using function from test55 project file
            //******************************************************************
        }
        s_bool3 = doesFileExist (cleanLibFile);
        if (s_bool3 == true) {remove ("libtable.dsv");}
        else {
            std::cout << "Step 5. Unable to create cleanlib.dsv." << std::endl;
            s_bool3 = false;
        }
    }

    // 6. If user configuration exists, MM.DB exists and songs table exists, process/update statistics: If user configuration exists, MM4 data exists,
    // songs table exists (bool1, bool2, bool3 are all true), run function to process/update statistics getDBStats()[NEED TO SPLIT FUNCTION
    // getCleanLib()]  (sets bool5)
    std::cout << "Step 6. if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == true)), get stats:"<< s_bool1 <<", s_bool2:"<< s_bool2 <<", s_bool3:"<< s_bool3 << std::endl;

    if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == true)) {
        //******************************************************************
        //getDBStats(); // need to add function getDBStats()
        //******************************************************************
        getDBStats(&s_rCode0TotTrackQty,&s_rCode0MsTotTime,&s_rCode1TotTrackQty,&s_rCode1MsTotTime,
                   &s_rCode3TotTrackQty,&s_rCode3MsTotTime,&s_rCode4TotTrackQty,&s_rCode4MsTotTime,
                   &s_rCode5TotTrackQty,&s_rCode5MsTotTime,&s_rCode6TotTrackQty,&s_rCode6MsTotTime,
                   &s_rCode7TotTrackQty,&s_rCode7MsTotTime,&s_rCode8TotTrackQty,&s_rCode8MsTotTime,
                   &s_SQL10TotTimeListened,&s_SQL10DayTracksTot,&s_SQL20TotTimeListened,
                   &s_SQL20DayTracksTot,&s_SQL30TotTimeListened,&s_SQL30DayTracksTot,&s_SQL40TotTimeListened,
                   &s_SQL40DayTracksTot,&s_SQL50TotTimeListened,&s_SQL50DayTracksTot,&s_SQL60TotTimeListened,
                   &s_SQL60DayTracksTot);

        // Need stat calculations for both needUpdate states
        // Convert variables from milliseconds to hours
        //  Total time in hours per rating code
        static int s_rCode0TotTime = (s_rCode0MsTotTime/60000)/60;
        static int s_rCode1TotTime = (s_rCode1MsTotTime/60000)/60;
        static int s_rCode3TotTime = (s_rCode3MsTotTime/60000)/60;
        static int s_rCode4TotTime = (s_rCode4MsTotTime/60000)/60;
        static int s_rCode5TotTime = (s_rCode5MsTotTime/60000)/60;
        static int s_rCode6TotTime = (s_rCode6MsTotTime/60000)/60;
        static int s_rCode7TotTime = (s_rCode7MsTotTime/60000)/60;
        static int s_rCode8TotTime = (s_rCode8MsTotTime/60000)/60;
        //  Total time listened in hours per rating code for each of six 10-day periods
        s_SQL10TotTimeListened = (s_SQL10TotTimeListened/60000)/60;
        s_SQL20TotTimeListened  = (s_SQL20TotTimeListened/60000)/60;
        s_SQL30TotTimeListened  = (s_SQL30TotTimeListened/60000)/60;
        s_SQL40TotTimeListened = (s_SQL40TotTimeListened/60000)/60;
        s_SQL50TotTimeListened = (s_SQL50TotTimeListened/60000)/60;
        s_SQL60TotTimeListened = (s_SQL60TotTimeListened/60000)/60;

        // Compile statistics and declare additional statistical variables
        //Total number of tracks in the library
        s_totalLibQty = s_rCode0TotTrackQty + s_rCode1TotTrackQty + s_rCode3TotTrackQty + s_rCode4TotTrackQty +
                s_rCode5TotTrackQty + s_rCode6TotTrackQty + s_rCode7TotTrackQty + s_rCode8TotTrackQty;
        s_totalRatedQty = s_totalLibQty - s_rCode0TotTrackQty; //Total number of rated tracks in the library
        s_totHrsLast60Days = s_SQL10TotTimeListened + s_SQL20TotTimeListened + s_SQL30TotTimeListened + s_SQL40TotTimeListened
                + s_SQL50TotTimeListened + s_SQL60TotTimeListened; //Total listened hours in the last 60 days
        // User listening rate weighted avg calculated using the six 10-day periods, and applying sum-of-the-digits for weighting
        s_listeningRate = ((s_SQL10TotTimeListened/10)*0.3) + ((s_SQL20TotTimeListened/10)*0.25)  + ((s_SQL30TotTimeListened/10)*0.2) +
                ((s_SQL40TotTimeListened/10)*0.15) + ((s_SQL50TotTimeListened/10)*0.1) + ((s_SQL60TotTimeListened/10)*0.05);

        static double s_adjHoursCode3 = (1 / s_yrsTillRepeatCode3) * s_rCode3TotTime;
        static double s_adjHoursCode4 = (1 / s_yrsTillRepeatCode4) * s_rCode4TotTime;
        static double s_adjHoursCode5 = (1 / s_yrsTillRepeatCode5) * s_rCode5TotTime;
        static double s_adjHoursCode6 = (1 / s_yrsTillRepeatCode6) * s_rCode6TotTime;
        static double s_adjHoursCode7 = (1 / s_yrsTillRepeatCode7) * s_rCode7TotTime;
        static double s_adjHoursCode8 = (1 / s_yrsTillRepeatCode8) * s_rCode8TotTime;
        static double s_totAdjHours = s_adjHoursCode3 + s_adjHoursCode4 + s_adjHoursCode5 + s_adjHoursCode6 +s_adjHoursCode7 + s_adjHoursCode8;
        s_ratingRatio3 = s_adjHoursCode3 / s_totAdjHours;
        s_ratingRatio4 = s_adjHoursCode4 / s_totAdjHours;
        s_ratingRatio5 = s_adjHoursCode5 / s_totAdjHours;
        s_ratingRatio6 = s_adjHoursCode6 / s_totAdjHours;
        s_ratingRatio7 = s_adjHoursCode7 / s_totAdjHours;
        s_ratingRatio8 = s_adjHoursCode8 / s_totAdjHours;

        s_DaysBeforeRepeatCode3 = s_yrsTillRepeatCode3 / 0.002739762; // fraction for one day (1/365)
        s_totalRatedTime = s_rCode1TotTime + s_rCode3TotTime + s_rCode4TotTime + s_rCode5TotTime + s_rCode6TotTime +
                s_rCode7TotTime + s_rCode8TotTime;
        static double s_AvgMinsPerSong = (s_totalRatedTime / s_totalRatedQty) * 60;
        static double s_avgListeningRateInMins = s_listeningRate * 60;
        s_SequentialTrackLimit = int((s_avgListeningRateInMins / s_AvgMinsPerSong) * s_DaysBeforeRepeatCode3);
        //static double s_STLF = 1 / s_SequentialTrackLimit;
        s_totalAdjRatedQty = (s_yrsTillRepeatCode3factor * s_rCode3TotTrackQty)+(s_yrsTillRepeatCode4factor * s_rCode4TotTrackQty)
                + (s_yrsTillRepeatCode5factor * s_rCode5TotTrackQty) +(s_yrsTillRepeatCode6factor * s_rCode6TotTrackQty)
                +(s_yrsTillRepeatCode7factor * s_rCode7TotTrackQty) + (s_yrsTillRepeatCode8factor * s_rCode8TotTrackQty);

        //Print results to console (for later program integration tasks)
        std::cout << "Total tracks Rating 0 - s_rCode0TotTrackQty : " << s_rCode0TotTrackQty << ". Total Time (hrs) - s_rCode0TotTime : " <<  s_rCode0TotTime << std::endl;
        std::cout << "Total tracks Rating 1 - s_rCode1TotTrackQty : " << s_rCode1TotTrackQty << ". Total Time (hrs) - s_rCode1TotTime : " <<  s_rCode1TotTime << std::endl;
        std::cout << "Total tracks Rating 3 - s_rCode3TotTrackQty : " << s_rCode3TotTrackQty << ". Total Time (hrs) - s_rCode3TotTime : " <<  s_rCode3TotTime << std::endl;
        std::cout << "Total tracks Rating 4 - s_rCode4TotTrackQty : " << s_rCode4TotTrackQty << ". Total Time (hrs) - s_rCode4TotTime : " <<  s_rCode4TotTime << std::endl;
        std::cout << "Total tracks Rating 5 - s_rCode5TotTrackQty : " << s_rCode5TotTrackQty << ". Total Time (hrs) - s_rCode5TotTime : " <<  s_rCode5TotTime << std::endl;
        std::cout << "Total tracks Rating 6 - s_rCode6TotTrackQty : " << s_rCode6TotTrackQty << ". Total Time (hrs) - s_rCode6TotTime : " <<  s_rCode6TotTime << std::endl;
        std::cout << "Total tracks Rating 7 - s_rCode7TotTrackQty : " << s_rCode7TotTrackQty << ". Total Time (hrs) - s_rCode7TotTime : " <<  s_rCode7TotTime << std::endl;
        std::cout << "Total tracks Rating 8 - s_rCode8TotTrackQty : " << s_rCode8TotTrackQty << ". Total Time (hrs) - s_rCode8TotTime : " <<  s_rCode8TotTime << std::endl;
        std::cout << "Total tracks in the library is - s_totalLibQty : " << s_totalLibQty << std::endl;
        std::cout << "Total rated tracks in the library is - s_totalRatedQty : " << s_totalRatedQty << std::endl;
        std::cout << "Total rated time in the library is - s_TotalRatedTime : " <<s_totalRatedTime << std::endl;
        std::cout << "Total time listened for first 10-day period is (hrs) - s_SQL10TotTimeListened : " << s_SQL10TotTimeListened << std::endl;
        std::cout << "Total tracks played in the first period is - s_SQL10DayTracksTot : " << s_SQL10DayTracksTot << std::endl;
        std::cout << "Total time listened for second 10-day period is (hrs) - s_SQL20TotTimeListened : " << s_SQL20TotTimeListened << std::endl;
        std::cout << "Total tracks played in the second period is - s_SQL20DayTracksTot : " << s_SQL20DayTracksTot << std::endl;
        std::cout << "Total time listened for third 10-day period is (hrs): " << s_SQL30TotTimeListened << std::endl;
        std::cout << "Total tracks played in the third period is - s_SQL30TotTimeListened : " << s_SQL30DayTracksTot << std::endl;
        std::cout << "Total time listened for fourth 10-day period is (hrs) - s_SQL40TotTimeListened : " << s_SQL40TotTimeListened << std::endl;
        std::cout << "Total tracks played in the fourth period is - s_SQL40DayTracksTot : " << s_SQL40DayTracksTot << std::endl;
        std::cout << "Total time listened for fifth 10-day period is (hrs) - s_SQL50TotTimeListened : " << s_SQL50TotTimeListened << std::endl;
        std::cout << "Total tracks played in the fifth period is - s_SQL50DayTracksTot : " << s_SQL50DayTracksTot << std::endl;
        std::cout << "Total time listened for sixth 10-day period is (hrs) - s_SQL60TotTimeListened : " << s_SQL60TotTimeListened << std::endl;
        std::cout << "Total tracks played in the sixth period is - s_SQL60DayTracksTot : " << s_SQL60DayTracksTot << std::endl;
        std::cout << "Total time listened for the last 60 days is (hrs) - s_totHrsLast60Days : " << s_totHrsLast60Days << std::endl;
        std::cout << "Calculated daily listening rate is (hrs) - s_listeningRate : "<< s_listeningRate << std::endl;
        std::cout << "Years between repeats code 3 - s_yrsTillRepeatCode3 : "<< s_yrsTillRepeatCode3 << std::endl;
        std::cout << "Years between repeats code 4 - s_yrsTillRepeatCode4 : "<< s_yrsTillRepeatCode4 << std::endl;
        std::cout << "Years between repeats code 5 - s_yrsTillRepeatCode5 : "<< s_yrsTillRepeatCode5 << std::endl;
        std::cout << "Years between repeats code 6 - s_yrsTillRepeatCode6 : "<< s_yrsTillRepeatCode6 << std::endl;
        std::cout << "Years between repeats code 7 - s_yrsTillRepeatCode7 : "<< s_yrsTillRepeatCode7 << std::endl;
        std::cout << "Years between repeats code 8 - s_yrsTillRepeatCode8 : "<< s_yrsTillRepeatCode8 << std::endl;
        std::cout << "Adjusted hours code 3 - s_adjHoursCode3 : "<< s_adjHoursCode3 << std::endl;
        std::cout << "Adjusted hours code 4 - s_adjHoursCode4 : "<< s_adjHoursCode4 << std::endl;
        std::cout << "Adjusted hours code 5 - s_adjHoursCode5 : "<< s_adjHoursCode5 << std::endl;
        std::cout << "Adjusted hours code 6 - s_adjHoursCode6 : "<< s_adjHoursCode6 << std::endl;
        std::cout << "Adjusted hours code 7 - s_adjHoursCode7 : "<< s_adjHoursCode7 << std::endl;
        std::cout << "Adjusted hours code 8 - s_adjHoursCode8 : "<< s_adjHoursCode8 << std::endl;
        std::cout << "Total Adjusted Hours - s_totAdjHours : "<< s_totAdjHours << std::endl;
        std::cout << "Total Adjusted Quantity - s_totalAdjRatedQty : "<< s_totalAdjRatedQty << std::endl;
        std::cout << "Percentage of track time for scheduling rating code 3 - s_ratingRatio3 * 100 : "<< s_ratingRatio3 * 100 << "%" << std::endl;
        std::cout << "Percentage of track time for scheduling rating code 4 - s_ratingRatio4 * 100 : "<< s_ratingRatio4  * 100 << "%" << std::endl;
        std::cout << "Percentage of track time for scheduling rating code 5 - s_ratingRatio5 * 100 : "<< s_ratingRatio5  * 100 << "%" << std::endl;
        std::cout << "Percentage of track time for scheduling rating code 6 - s_ratingRatio6 * 100 : "<< s_ratingRatio6 * 100 <<  "%" << std::endl;
        std::cout << "Percentage of track time for scheduling rating code 7 - s_ratingRatio7 * 100 : "<< s_ratingRatio7 * 100 <<  "%" << std::endl;
        std::cout << "Percentage of track time for scheduling rating code 8 - s_ratingRatio8 * 100 : "<< s_ratingRatio8 * 100 <<  "%" << std::endl;
        std::cout << "Number of days until track repeat under rating code 3 - s_DaysBeforeRepeatCode3 : "<< s_DaysBeforeRepeatCode3 << std::endl;
        std::cout << "Average length of rated songs in fractional minutes - s_AvgMinsPerSong : "<< s_AvgMinsPerSong << std::endl;
        std::cout << "Calculated daily listening rate in mins - s_avgListeningRateInMins : "<< s_avgListeningRateInMins << std::endl;
        std::cout << "Calculated tracks per day - s_avgListeningRateInMins / s_AvgMinsPerSong : "<< s_avgListeningRateInMins / s_AvgMinsPerSong << std::endl;
        std::cout << "Sequential Track Limit - s_SequentialTrackLimit : "<< s_SequentialTrackLimit << std::endl;

        s_bool5 = true;
    }
    else {
        std::cout << "Step 6. Something went wrong at function getDBStats." << std::endl;
        s_bool5 = false;
    }


    std::cout << "If bool3 = false (songs table exists), skipping step 7. Bool3:"<<s_bool3 << std::endl;


    // 7. If user configuration exists, MM.DB exists, songs table exists, and statistics are processed, generate artist statistics:
    // If user configuration exists, MM4 data exists, songs table exists, and database statistics exists (bool1, bool2, bool3, bool5
    // are all true), run function getArtistAdjustedCount() to generate artist statistics (it creates the file artistsadj.txt, with adjusted
    // counts by rating and artist intervals for each track) [ need to revise getArtistAdjustedCount() function to change the read
    // file from rated.dsv to cleanlib.dsv ] then set bool8 to true, rechecking, run doesFileExist (const std::string& name)
    // (artistsadj.txt) function (bool10)
    std::cout << "Step 7. if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == true) && (s_bool5 == true)), get artist stats:"<< s_bool1 <<", s_bool2:"<< s_bool2 <<", s_bool3:"<< s_bool3 << std::endl;


    //******************************************************************
    // need to add function getArtistAdjustedCount2()
    //******************************************************************

    if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == true) && (s_bool5 == true)) {
        getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                               &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                               &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                               &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);
        s_bool10 = doesFileExist ("artistsadj.txt");
        if (s_bool10 == false)  {std::cout << "Step 7. Something went wrong at the function getArtistAdjustedCount. artistsadj.txt not created." << std::endl;}
    }


    // 8.  If user configuration exists, MM.DB exists, songs table exists, database statistics exist, artist statistics are processed, create
    // a modified database with only rated tracks and which include artist intervals calculated for each: If user configuration exists,
    // MM4 data exists, songs table exists, database statistics exist, and file artistsadj.txt is created (bool1, bool2, bool3, bool5, bool10
    // are all true), run function buildDB()  [  see test56excludes for test version ] to create a modified database file with rated tracks
    // only and artist intervals for each track, rechecking, run doesFileExist (const std::string& name) (ratedabbr.txt) function (bool11)
    std::cout << "Step 8. if s_bool1 s_bool2 s_bool3 s_bool5 and s_bool10 are true, buidl AS db" << std::endl;


    //******************************************************************
    // need to add function buildDB()  see test56excludes for test version
    //******************************************************************
    if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == true) && (s_bool5 == true) && (s_bool10 == true)) {
        buildDB();
        s_bool11 = doesFileExist ("ratedabbr.txt");
        if (s_bool10 == false)  {std::cout << "Step 8. Something went wrong at the function buildDB(). ratedabbr.txt not created." << std::endl;}
    }

    // 9. Determine if a playlist was selected, and if it was not, determine if it was identified as being selected in the user's config:
    // Determine if cleaned (path-corrected) playlist selected (bool6) cleanedplaylist.txt exists, doesFileExist (const std::string& name);
    // sets bool6 and bool7

    s_bool6 = doesFileExist (cleanedPlaylist);
    if (s_bool6 == true) {s_bool7 = true;}
    //a. If bool6 is false, determine if playlist was identified as selected in user config (sets bool7)
    if (s_bool6 == false){
        //getConfigEntry: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
        std::string s_selectedplaylist = userconfig::getConfigEntry(7);
        if (s_selectedplaylist != "") {s_bool7 = true;}
    }

    // 10. If a playlist was identified in the user config, but the playlist file is not found, obtain the playlist file: If user configuration
    // exists, MM4 data exists, songs table exists (bool1, bool2, bool3 are all true), and playlist from user config exists (bool7 is true),
    // but cleaned playlist does not (bool6 is false), run function to obtain cleaned playlist file getPlaylist() then set bool6 to true,
    // rechecking, run doesFileExist (const std::string& name) function. Evaluates bool6 and sets to true (after running getPlaylist) if
    // initially false

    if ((s_bool1 == true) && (s_bool2 == true) && (s_bool3 == true) && (s_bool7 == true) && (s_bool6 == false))
    {
        getPlaylist();
        s_bool6 = doesFileExist (cleanedPlaylist);
        if (s_bool6 == false) {std::cout << "Step 10. Something went wrong at the function getPlaylist." << std::endl;}
    }

    // NOTE: functions used in the next  three steps (11-13) will later be reused when adding tracks to
    // playlist - here, this is to get the initial values if a playlist exists

    //11. If playlist exists, calculate the playlist size: If cleaned playlist exists (bool6 is true), obtain playlist size
    // using function cstyleStringCount(),  s_playlistSize = cstyleStringCount(cleanedPlaylist);

    if (s_bool6 == true) {s_playlistSize = cstyleStringCount(cleanedPlaylist);}

    // 12. If playlist exists, obtain the historical count (in addition to the playlist count) up to the sequential track limit:
    // If cleaned playlist exists (bool6 is true), obtain the historical count (in addition to the playlist count) up to the
    // sequential track limit. A variable is needed (which later will be used to obtain additional play history outside of
    // playlist, as part of a later function to make a new track selection), using the variable s_histCount. The value is
    // calculated [ can be modified to use the function  to added function void getHistCount(&s_SequentialTrackLimit,&s_playlistSize),
    // or just: s_histCount = long(s_SequentialTrackLimit) – long(s_playlistSize); this uses both playlist size from 10
    // and SequentialTrackLimit obtained with data from function getDBStats()]
    if (s_bool6 == true) {s_playlistSize = cstyleStringCount(cleanedPlaylist);
        s_histCount = s_SequentialTrackLimit - s_playlistSize;}

    //13. If playlist exists, artist statistics are processed, and modified database exists, create/update excluded artists
    // list: If cleaned playlist exists (bool6 is true), and artistsadj.txt exists (bool8 is true) and modified database exists
    // (bool11), run [need to write new] function getExcludedArtists()  to create/update excluded artists list using vectors
    // read in from the following files: cleanlib.dsv, artistsadj.txt, and cleanedplaylist.txt. Writes artistexcludes.txt. Also,
    // creates temporary database (ratedabbr2.txt) with playlist position numbers for use in subsequent functions,
    //ratingCodeSelected and selectTrack

    if ((s_bool6 == true) && (s_bool8 == true) && (s_bool11 == true))  {
        getExcludedArtists(&s_histCount, &s_playlistSize);

    }
}



void ArchSimian::on_addsongsButton_clicked(){
    // Add loop here:  For each number of songs selected, run...
    s_ratingNextTrack = ratingCodeSelected(&s_ratingRatio3,&s_ratingRatio4,&s_ratingRatio5,&s_ratingRatio6,&s_ratingRatio7,&s_ratingRatio8);
    std::cout << "Rating for the next track is " << s_ratingNextTrack << std::endl;
    selectTrack(&s_ratingNextTrack);
    s_playlistSize = cstyleStringCount("cleanedplaylist.txt");
    std::cout << "Playlist length is: " << s_playlistSize << " tracks." << std::endl;
    s_histCount = long(s_SequentialTrackLimit) - long(s_playlistSize);
    getExcludedArtists(&s_histCount, &s_playlistSize);
    s_ratingNextTrack = ratingCodeSelected(&s_ratingRatio3,&s_ratingRatio4,&s_ratingRatio5,&s_ratingRatio6,&s_ratingRatio7,&s_ratingRatio8);
}

void ArchSimian::on_exportplaylistButton_clicked(){

}

void ArchSimian::on_setlibraryButton_clicked(){
    QFileDialog setlibraryButton;
    //    if( !setlibraryButton.exec() )
    //    {
    // The user pressed the cancel button so handle this accordingly
    //        return;
    //    }
    //    else {
    setlibraryButton.setFileMode(QFileDialog::Directory);
    setlibraryButton.setOption(QFileDialog::ShowDirsOnly);
    const QString s_musiclibrarydirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select Shared Music Library Directory"),
                "/"
                );
    ui->setlibrarylabel->setText(QString(s_musiclibrarydirname));
    // Write description note and directory configuration to archsimian.conf
    std::ofstream userconfig(Constants::userFileName);
    std::string str("# Location of music library");
    userconfig << str << "\n";  // Write to line 1, archsimian.conf
    str = s_musiclibrarydirname.toStdString();
    userconfig << str << "\n";  // Write to line 2, archsimian.conf
    userconfig.close();
    // dim the setlibraryButton button
    ui->setlibraryButton->setEnabled(false);
    // Enable the reset button
    ui->setlibraryButtonReset->setVisible(true);
    // Activate the second of three config buttons
    ui->setmmplButton->setEnabled(true);
    //      }
}

void ArchSimian::on_setmmplButton_clicked(){
    QFileDialog setmmpldialog;
    //       if( !setmmpldialog.exec() )
    //        {
    // The user pressed the cancel button so handle this accordingly
    //           return;
    //       }
    //       else {
    setmmpldialog.setFileMode(QFileDialog::Directory);
    setmmpldialog.setOption(QFileDialog::ShowDirsOnly);
    const QString s_mmbackuppldirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select MediaMonkey Playlist Backup Directory"),
                "/"
                );
    ui->setmmpllabel->setText(QString(s_mmbackuppldirname));
    // Write description note and directory configuration to archsimian.conf
    std::ofstream userconfig(Constants::userFileName, std::ios::app);
    std::string str("# Location of MediaMonkey Playlist Backup Directory");
    userconfig << str << "\n";  // Write to line 3, archsimian.conf
    str = s_mmbackuppldirname.toStdString();
    userconfig << str << "\n"; // Write to line 4, archsimian.conf
    userconfig.close();
    //dim the setmmplButton button
    ui->setmmplButton->setEnabled(false);
    //enable the reset button
    ui->setmmplButtonReset->setVisible(true);
    // Activate the last of three config buttons
    ui->setmmdbButton->setEnabled(true);
}

void ArchSimian::on_setmmdbButton_clicked(){
    QFileDialog setmmdbdialog;
    //       if( !setmmdbdialog.exec() )
    //       {
    // The user pressed the cancel button so handle this accordingly
    //           return;
    //        }
    //       else {
    setmmdbdialog.setFileMode(QFileDialog::Directory);
    setmmdbdialog.setOption(QFileDialog::ShowDirsOnly);
    const QString s_mmbackupdbdirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select MediaMonkey Database (MM.DB) Backup Directory"),
                "/"
                );
    ui->setmmdblabel->setText(QString(s_mmbackupdbdirname));
    // Write description note and directory configuration to archsimian.conf
    std::ofstream userconfig(Constants::userFileName, std::ios::app);
    std::string str("# Location of MediaMonkey Database Backup Directory");
    userconfig << str << "\n";  // Write to line 5, archsimian.conf
    str = s_mmbackupdbdirname.toStdString();
    userconfig << str << "\n"; // Write to line 6, archsimian.conf
    userconfig.close();
    //dim the setmmdbButton button
    ui->setmmdbButton->setEnabled(false);
    //enable the reset button
    ui->setmmdbButtonReset->setVisible(true);
}

void ArchSimian::on_setlibraryButtonReset_clicked()
{
    //dim the reset button

    //enable the current button
}

void ArchSimian::on_setmmplButtonReset_clicked()
{
    //dim the reset button
    //enable the current button
}

void ArchSimian::on_setmmdbButtonReset_clicked()
{
    //dim the reset button
    //enable the current button
}

// User selects playlist from configured directory for 'backup playlists'
void ArchSimian::on_getplaylistButton_clicked()
{
    QFileDialog setgetplaylistdialog;
    QString selectedplaylist = QFileDialog::getOpenFileName (
                this,
                "Select playlist for which you will add tracks",
                QString::fromStdString(userconfig::getConfigEntry(3)),//default dir for playlists
                "playlists(.m3u) (*.m3u)");
    ui->setgetplaylistLabel->setText("Selected: " + QString(selectedplaylist));
    // Write description note and playlist name to archsimian.conf
    std::ofstream userconfig(Constants::userFileName, std::ios::app);
    std::string str("# Name of default playlist");
    userconfig << str << "\n";  // Write to line 7, archsimian.conf
    str = selectedplaylist.toStdString();
    userconfig << str << "\n"; // Write to line 8, archsimian.conf
    userconfig.close();
}

void ArchSimian::on_mainQTabWidget_tabBarClicked(int index)
{
    if (index == 2) // if the Statistics tab is selected, refresh stats
    {
        ;
        ui->ybrLabel3->setText("Years between repeats, rating code 3: " + QString::fromStdString(std::to_string(s_yrsTillRepeatCode3)) +
                               " (in days: " + QString::fromStdString(std::to_string(int(s_daysTillRepeatCode3))) + ")");
        ui->ybrLabel4->setText("Years between repeats, rating code 4: " + QString::fromStdString(std::to_string(s_yrsTillRepeatCode4)));
        ui->ybrLabel5->setText("Years between repeats, rating code 5: " + QString::fromStdString(std::to_string(s_yrsTillRepeatCode5)));
        ui->ybrLabel6->setText("Years between repeats, rating code 6: " + QString::fromStdString(std::to_string(s_yrsTillRepeatCode6)));
        ui->ybrLabel7->setText("Years between repeats, rating code 7: " + QString::fromStdString(std::to_string(s_yrsTillRepeatCode7)));
        ui->ybrLabel8->setText("Years between repeats, rating code 8: " + QString::fromStdString(std::to_string(s_yrsTillRepeatCode8)));

        ui->totadjtracksLabel->setText(tr("Current playlist size is: ") + QString::fromStdString(std::to_string(s_playlistSize)));
        ui->tottracksLabel->setText("Total tracks in the library is: " + QString::fromStdString(std::to_string(s_totalLibQty)));
        ui->totratedtracksLabel->setText("Total rated tracks in the library is: " + QString::fromStdString(std::to_string(s_totalRatedQty)));
        ui->totratedtimeLabel->setText("Total rated time (in hours) is: " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
        ui->dailylistenLabel->setText("Calculated daily listening rate (in hours) is: " + QString::fromStdString(std::to_string(s_listeningRate)));

    }
}

//void ArchSimian::on_pushButton_clicked()
//{

//}

void ArchSimian::on_refreshdbButton_clicked()
{

    // Add code to 'goto' beginning of this cpp, then delete the rest of the below code except last few lines?





    s_LastTableDate = getLastTableDate(); // function is in dependents.cpp
    ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(s_MMdbDate)+ tr(" Library date: ")+ QString::fromStdString(s_LastTableDate));
    //ui->updatestatusLabel->setText("Library update completed.");
}
