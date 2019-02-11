//archsimian.cpp is equivalent to MainWindow.cpp. This and archsimian.h are
// where the UI code resides. The main () function is located at main.cpp

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstddef>
#include "archsimian.h"
#include "constants.h"
#include "ui_archsimian.h"
#include "userconfig.h"
#include "dependents.h"
#include "getplaylist.h"
#include "writesqlfile.h"
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
static bool s_bool_IsUserConfigSet {false};
static bool s_bool_MMdbExist{false};
static bool s_bool_CleanLibExist{false};
static bool s_bool_MMdbUpdated{false};
static bool s_bool_dbStatsCalculated{false};
static bool s_bool_artistsadjExist{false};
static bool s_bool_RatedAbbrExist{false};
static bool s_bool_PlaylistExist{false};
static bool s_bool_PlaylistSelected{false};
static bool s_bool_ExcludedArtistsProcessed{false};

static QString s_mmBackupDBDir{""};
static QString s_musiclibrarydirname{""};
static QString mmPlaylistDir{""};
//static std::string s_mmbackuppldirname{""};
//static std::string s_selectedplaylist{""};
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
static double s_AvgMinsPerSong{0.0};
static double s_avgListeningRateInMins{0.0};
//static int s_repeatFreqForCode1{20};
static int s_dateTranslation{12};
static QString dateTransTextVal{" months"};
static double sliderBaseVal3{0.0};
static std::string s_selectedTrackPath{""};

ArchSimian::ArchSimian(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ArchSimian)
{
    loadSettings(); // load user settings
    s_daysTillRepeatCode3 = m_prefs.s_daysTillRepeatCode3;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    //std::cout << "LoadSettings, s_repeatFactorCode4 result: "<< s_repeatFactorCode4<<" and s_yrsTillRepeatCode4 result: "<< s_yrsTillRepeatCode4<<std::endl;
    s_repeatFactorCode5 = m_prefs.s_repeatFactorCode5;
    s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
    s_repeatFactorCode6 = m_prefs.s_repeatFactorCode6;
    s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
    s_repeatFactorCode7 = m_prefs.s_repeatFactorCode5;
    s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
    s_repeatFactorCode8 = m_prefs.s_repeatFactorCode6;
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    sliderBaseVal3 = m_prefs.s_daysTillRepeatCode3 / 365;
    s_mmBackupDBDir = m_prefs.mmBackupDBDir;
    m_prefs.musicLibraryDir = s_musiclibrarydirname;
    //
// Step 1. Determine if user configuration exists:  Run isConfigSetup() function (s_bool_IsUserConfigSet)
    //
    // UI configuration: set default state to "false" for user config reset buttons
    ui->setupUi(this);
    //connect(ui->repeatFreq1SpinBox,SIGNAL(valueChanged)),ui->addsongsButton,SLOT(setValue(s_repeatFreqForCode1))
    ui->mainQTabWidget->setCurrentIndex(0);



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
    else {
        s_bool_IsUserConfigSet = false;
        std::cout << "Step 1. Unable to open archsimian.conf configuration file or file has no data. s_bool_IsUserConfigSet result: "<< s_bool_IsUserConfigSet<<std::endl;
    }
    //If configuration has already been set, populate the ui labels accordingly
    if (archsimianconfsize != 0)
    {
        s_bool_IsUserConfigSet = true;
        if (Constants::verbose == true) std::cout << "Step 1. Configuration has already been set. s_bool_IsUserConfigSet result: "<<s_bool_IsUserConfigSet<<std::endl;
        //ui->setCurrentIndex(0);
        m_prefs.musicLibraryDir = s_musiclibrarydirname;
        ui->setlibrarylabel->setText(QString(s_musiclibrarydirname));
        //dim the setlibraryButton button
        ui->setlibraryButton->setEnabled(true);
        //enable the reset button
        //s_mmbackuppldirname = userconfig::getConfigEntry(3);
        ui->setmmpllabel->setText(QString(m_prefs.defaultPlaylist));
        //dim the setmmplButton button
        ui->setmmplButton->setEnabled(true);
        //enable the reset button
        //std::string s_mmbackupdbdirname = userconfig::getConfigEntry(5);
        ui->setmmdblabel->setText(m_prefs.mmBackupDBDir);
        //std::string selectedplaylist = userconfig::getConfigEntry(7);
        ui->setgetplaylistLabel->setText("Selected: " + m_prefs.defaultPlaylist);
        //dim the setmmdbButton button
        ui->setmmdbButton->setEnabled(true);
        //enable the reset button


        //****************************************** Consider moving this section to step 4

        //
        // The next section is apparently for updating the UI for DB update status - it is a different (older) function than the new one
        //

        bool needUpdate = recentlyUpdated(s_mmBackupDBDir);

        //bool needUpdate = isLibRefreshNeeded(); // function isLibRefreshNeeded() is from dependents.cpp
        if (Constants::verbose == true) std::cout << "Step 1. Checking isLibRefreshNeeded(): "<<needUpdate<<std::endl;
        if (needUpdate == 0)
        {
            s_MMdbDate = getMMdbDate();
            s_LastTableDate = getLastTableDate();
            ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(s_MMdbDate)+
                                           tr(", Library date: ")+ QString::fromStdString(s_LastTableDate));
            // dim update library button
            //ui->refreshdbButton->setEnabled(false);
        }
    }

    else {  // Otherwise, configuration has not been set. Load instructions for user to locate and set config
        // Initially, only the first of three config buttons is activated. Build the file in sequence.
        s_bool_IsUserConfigSet = false;
        ui->setlibrarylabel->setText(tr("Select the base directory of "
                                        "your music library"));
        ui->setlibraryButton->setEnabled(true);
        ui->setmmpllabel->setText(tr("Select the shared Windows directory"
                                     " where you manually exported your playlist(s) from MediaMonkey"));
        ui->setmmplButton->setEnabled(true);
        ui->setmmdblabel->setText(tr("Select the shared Windows directory"
                                     " where you stored the MediaMonkey database backup file (MM.DB)"));
        ui->setmmdbButton->setEnabled(false);
        ui->setgetplaylistLabel->setText(tr("Select playlist for adding tracks"));
    }

    //******************************************

// Step 2. Determine if MM.DB database file exists: Run doesFileExist (const std::string& name) function (sets s_bool_MMdbExist).

    std::string mmdbdir = userconfig::getConfigEntry(5); // z: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath);
    const std::string mmpath = mmdbdir + "/MM.DB";
    s_bool_MMdbExist = doesFileExist(mmpath);

    //    a. If s_boolIsUserConfigSet is true, but s_bool_MMdbExist is false, report to user that MM.DB was not found at the
    //       location specified and set s_bool_IsUserConfigSet to false
    //    b. If s_boolIsUserConfigSet is false, set s_bool_MMdbExist to false


    if (Constants::verbose == true) std::cout << "Step 2. Does MM.DB file exist. s_bool_MMdbExist result: "<< s_bool_MMdbExist << std::endl;

    if (s_bool_IsUserConfigSet == false) {
        s_bool_MMdbExist = false;
        s_bool_MMdbUpdated = false;
        s_bool_PlaylistExist = false;
    }
    if ((s_bool_IsUserConfigSet == true) && (s_bool_MMdbExist == false)) {
        if (Constants::verbose == true) std::cout << "Step 2. MM.DB was not found at the location you specified. Setting s_bool_IsUserConfigSet to false." << std::endl;
        s_bool_IsUserConfigSet = false;
    }

// Step 3. Determine if Archsimian songs table exists: If user configuration exists and MM4 data exists (s_bool_IsUserConfigSet and s_bool_MMdbExist are true),
    //determine if cleanlib.dsv songs table exists in AS, function doesFileExist (const std::string& name)  (dir paths corrected,
    // imported from MM.DB) (sets s_bool_CleanLibExist)

    if ((s_bool_IsUserConfigSet == true) && (s_bool_MMdbExist == true)) {
        bool tmpbool;
        tmpbool = doesFileExist(Constants::cleanLibFile);
        if (tmpbool == true){ // check that file is not empty
            //Check whether the songs table currently has any data in it
            std::streampos cleanLibFilesize;
            char * memblock;
            std:: ifstream file (Constants::cleanLibFile, std::ios::in|std::ios::binary|std::ios::ate);
            if (file.is_open())
            {
                cleanLibFilesize = file.tellg();
                memblock = new char [cleanLibFilesize];
                file.seekg (0, std::ios::beg);
                file.read (memblock, cleanLibFilesize);
                file.close();
                delete[] memblock;
            }

            if (cleanLibFilesize != 0) {s_bool_CleanLibExist = true;}//doesFileExist(cleanLibFile);
        }
    }
    if (Constants::verbose == true) std::cout << "Step 3. Does CleanLib file exist. s_bool_CleanLibExist result: "
                                              << s_bool_CleanLibExist << std::endl;

    // 4. Determine if MM.DB was recently updated: s_bool_MMdbUpdated is set by comparing MM.DB file date
    // to CleanLib (songs table) file date. If the MM.DB file date is newer (greater) than the CleanLib file date
    // will need to be updated.

    s_bool_MMdbUpdated = recentlyUpdated(s_mmBackupDBDir);
    if (Constants::verbose == true) std::cout << "Step 4. Is the MM.DB file date newer (greater) than the CleanLib file date."
                                                 " s_bool_MMdbUpdated result: "<< s_bool_MMdbUpdated << std::endl;
    // set ui labels if MM.DB was recently updated
    if (s_bool_MMdbUpdated == true) // bool s_bool_MMdbUpdated: 1 means refresh DB, 0 means skip
    // If result is 1, remove ratedabbr.txt, ratedabbr2.txt, artistsadj.txt, playlistposlist.txt, artistexcludes.txt, and cleanedplaylist.txt files
    {
        remove("ratedabbr.txt");
        s_bool_RatedAbbrExist = false;
        remove("ratedabbr2.txt");
        remove("artistsadj.txt");
        s_bool_artistsadjExist = false;
        remove("playlistposlist.txt");
        remove("artistexcludes.txt");
        s_bool_ExcludedArtistsProcessed = false;
        remove("cleanedplaylist.txt");
        s_bool_PlaylistExist = false;
        //ui->refreshdbButton->setEnabled(true);
        ui->updatestatusLabel->setText(tr("MM.DB was recently backed up. Library has been rebuilt."));}

// Step 5. If user configuration and MM4 data exist, but the songs table does not, import songs table into Archsimian: If user configuration
    // and MM4 data exist, but the songs table does not (bool_IsUserConfigSet, s_bool_MMdbExist are true, s_bool_CleanLibExist is false), import songs table into AS, by running
    // writeSQLFile() function, which creates the temporary basic table file libtable.dsv; then run the getLibrary() function, which creates
    // the refined table file cleanlib.dsv The getLibrary() function completes the
    // following refinements: (a) corrects the directory paths to Linux, (b) adds random lastplayed dates for rated or "new-need-to-be-rated"
    // tracks that have no play history, (c) creates rating codes for any blank values found in GroupDesc col, using POPM values,
    // and (d) creates Artist codes (using col 1) and places the code in Custom2 if Custom2 is blank. Then set s_bool_CleanLibExist to true,  rechecking,
    // run doesFileExist (const std::string& name) function. After verifying  cleanlib.dsv exists, remove temporary basic table file
    // libtable.dsv Evaluates s_bool_CleanLibExist for existence of cleanlib.dsv (cleanLibFile)

    if ((Constants::verbose == true)&&(s_bool_MMdbUpdated == false)&&(s_bool_CleanLibExist == true)){
        std::cout << "Step 5. CleanLib file exists and MM.DB was not recently updated. Skip to Step 6."<< std::endl;}

    if (((s_bool_IsUserConfigSet == true) && (s_bool_MMdbExist == true) && (s_bool_CleanLibExist == false)) || (s_bool_MMdbUpdated == true)) {
        if (Constants::verbose == true) std::cout << "Step 5. User configuration and MM.DB exists, but the songs table does not, or MM.DB was"
                                                     "recently updated. Importing songs table (create CleanLib) into Archsimian from MM.DB..." <<std::endl;
        writeSQLFile();
        pid_t c_pid;// Create fork object; child to get database table into a dsv file, then child to open that table only
        // after it finishes getting written, not before.
        c_pid = fork(); // Run fork function
        int status; // For status of pid process
        if( c_pid == 0 ){ // Child process: Get songs table from MM4 database, and create libtable.dsv with table;
            //std::string s_mmbackupdbdirname = userconfig::getConfigEntry(5); // 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
            // revise for QStandardPaths class if this does not set with makefile for this location
            const std::string sqlpathdirname = getenv("HOME");
            s_mmBackupDBDir = m_prefs.mmBackupDBDir + "/MM.DB";
            std::string tmpDBdir = s_mmBackupDBDir.toStdString();
            std::string path2 = ".read " + sqlpathdirname + "/exportMMTable.sql";
            const char* const argv[] = {" ", tmpDBdir.c_str(), path2.c_str(), nullptr};
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
            getLibrary(s_musiclibrarydirname); // get songs table from MM.DB
        }
        s_bool_CleanLibExist = doesFileExist (Constants::cleanLibFile);
        if (s_bool_CleanLibExist == true) {remove ("libtable.dsv");}
        else {
            std::cout << "Step 5. Unable to create cleanlib.dsv." << std::endl;
            s_bool_CleanLibExist = false;
        }
    }

// Step 6. If user configuration exists, MM.DB exists and songs table exists, process/update statistics: If user configuration exists, MM4 data exists,
    // songs table exists (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist are all true), run function to process/update statistics getDBStats()

    if (Constants::verbose == true) std::cout << "Step 6. User configuration exists, MM.DB exists and songs table exists. Processing database statistics:" << std::endl;

    if ((s_bool_IsUserConfigSet == true) && (s_bool_MMdbExist == true) && (s_bool_CleanLibExist == true)) {
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
        s_AvgMinsPerSong = (s_totalRatedTime / s_totalRatedQty) * 60;
        s_avgListeningRateInMins = s_listeningRate * 60;
        s_SequentialTrackLimit = int((s_avgListeningRateInMins / s_AvgMinsPerSong) * s_DaysBeforeRepeatCode3);
        //static double s_STLF = 1 / s_SequentialTrackLimit;
        s_totalAdjRatedQty = (s_yrsTillRepeatCode3factor * s_rCode3TotTrackQty)+(s_yrsTillRepeatCode4factor * s_rCode4TotTrackQty)
                + (s_yrsTillRepeatCode5factor * s_rCode5TotTrackQty) +(s_yrsTillRepeatCode6factor * s_rCode6TotTrackQty)
                +(s_yrsTillRepeatCode7factor * s_rCode7TotTrackQty) + (s_yrsTillRepeatCode8factor * s_rCode8TotTrackQty);

        //Print verbose results to console
        if (Constants::verbose == true) {
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
        std::cout << "Sequential Track Limit - s_SequentialTrackLimit : "<< s_SequentialTrackLimit << std::endl<< std::endl;
        }
        s_bool_dbStatsCalculated = true; // Set bool to true for s_bool_dbStatsCalculated
        ui->daystracksLabel->setText(QString::number((50 * s_AvgMinsPerSong)/s_avgListeningRateInMins,'g', 3));//s_listeningRate //double(s_AvgMinsPerSong*value)/s_avgListeningRateInMins)

    }
    else {
        s_bool_dbStatsCalculated = false;
        std::cout << "Step 6. Something went wrong at function getDBStats." << std::endl;
    }

// Step 7a. If user configuration exists, MM.DB exists, songs table exists, statistics are processed, and
    //MM.DB was not recently updated, check for existence of s_bool_artistsadjExist (artistsadj.txt).
    //If file is missing or empty, create file with artist statistics

    if ((s_bool_IsUserConfigSet == true) && (s_bool_MMdbExist == true)&& (s_bool_CleanLibExist == true)&&
            (s_bool_dbStatsCalculated == true)&&(s_bool_MMdbUpdated == false)) {
        s_bool_artistsadjExist = false;
        bool tmpbool;
        bool tmpbool2;
        tmpbool = doesFileExist("artistsadj.txt");
        if (tmpbool == true){ // check that file is not empty
            //Check whether the songs table currently has any data in it
            std::streampos artsistAdjsize;
            char * memblock;
            std:: ifstream file ("artistsadj.txt", std::ios::in|std::ios::binary|std::ios::ate);
            if (file.is_open())
            {
                artsistAdjsize = file.tellg();
                memblock = new char [artsistAdjsize];
                file.seekg (0, std::ios::beg);
                file.read (memblock, artsistAdjsize);
                file.close();
                delete[] memblock;
            }
            if (artsistAdjsize != 0) {s_bool_artistsadjExist = true;// file artistsadj.txt exists and is greater in size than zero, set to true
                // If MM.DB not recently updated and artistsadj.txt does not need to be updated, check if ratedabbr.txt exists
                // If it does set s_bool_RatedAbbrExist to true.
                if (Constants::verbose == true) {std::cout << "Step 7. MM.DB not recently updated and artistsadj.txt does not need to be updated. Now checking s_bool_RatedAbbrExist." << std::endl;}
                tmpbool2 = doesFileExist("ratedabbr.txt");
                if (tmpbool2 == true){ // check that file is not empty
                    //Check whether the songs table currently has any data in it
                    std::streampos ratedabbrsize;
                    char * memblock;
                    std:: ifstream file ("ratedabbr.txt", std::ios::in|std::ios::binary|std::ios::ate);
                    if (file.is_open())
                    {
                        ratedabbrsize = file.tellg();
                        memblock = new char [ratedabbrsize];
                        file.seekg (0, std::ios::beg);
                        file.read (memblock, ratedabbrsize);
                        file.close();
                        delete[] memblock;
                    }
                    if (ratedabbrsize != 0) {
                        s_bool_RatedAbbrExist = true;
                        if (Constants::verbose == true) {std::cout << "Step 7. Set s_bool_RatedAbbrExist = true." << std::endl;}
                    }
                }
            }
            if (artsistAdjsize == 0) {s_bool_artistsadjExist = false;}// file exists but size is zero, set to false
        }
        if (tmpbool == false){s_bool_artistsadjExist = false;} // file does not exist, set bool to false

        if (Constants::verbose == true) std::cout << "Step 7. MM.DB not recently updated. Verifying artistsadj.txt exists and is not zero. "
                                                     "s_bool_artistsadjExist result: "<< s_bool_artistsadjExist << std::endl;
        if (s_bool_artistsadjExist == false){
            getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                                   &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                                   &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                                   &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);
            s_bool_artistsadjExist = doesFileExist ("artistsadj.txt");
            s_bool_RatedAbbrExist = false;
            if (s_bool_artistsadjExist == false)  {std::cout << "Step 7(a) Something went wrong at the function getArtistAdjustedCount. artistsadj.txt not created." << std::endl;}
        }
    }

// Step 7b. If user configuration exists, MM.DB exists, songs table exists, statistics are processed, and
   // MM.DB was recently updated, create file with artist statistics

    if ((s_bool_IsUserConfigSet == true) && (s_bool_MMdbExist == true)&& (s_bool_CleanLibExist == true)&&
            (s_bool_dbStatsCalculated == true)&&(s_bool_MMdbUpdated == true)) {
        if (Constants::verbose == true) {std::cout << "Step 7. MM.DB was recently updated. Processing artist statistics..." << std::endl;}
        getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                               &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                               &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                               &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);
        s_bool_RatedAbbrExist = false;
        s_bool_artistsadjExist = doesFileExist ("artistsadj.txt");
        if (s_bool_artistsadjExist == false)  {std::cout << "Step 7(b). Something went wrong at the function getArtistAdjustedCount. artistsadj.txt not created." << std::endl;}
    }

    // 8.  If user configuration exists, MM.DB exists, songs table exists, database statistics exist, artist statistics are processed, create
    // a modified database with only rated tracks and which include artist intervals calculated for each: If user configuration exists,
    // MM4 data exists, songs table exists, database statistics exist, and file artistsadj.txt is created (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist,
    // s_bool_dbStatsCalculated, bool10 are all true), run function buildDB() to create a modified database file with rated tracks
    // only and artist intervals for each track, rechecking, run doesFileExist (const std::string& name) (ratedabbr.txt) function (s_bool_RatedAbbrExist)


    if ((s_bool_IsUserConfigSet == true) && (s_bool_MMdbExist == true) && (s_bool_CleanLibExist == true)  && (s_bool_dbStatsCalculated == true)
            && (s_bool_artistsadjExist == true) && (s_bool_RatedAbbrExist == false)) {
        buildDB();
        s_bool_RatedAbbrExist = doesFileExist ("ratedabbr.txt");
        if (s_bool_RatedAbbrExist == false)  {std::cout << "Step 8. Something went wrong at the function buildDB(). ratedabbr.txt not created." << std::endl;}
        if ((s_bool_RatedAbbrExist == true)&&(Constants::verbose == true)){std::cout << "Step 8. ratedabbr.txt was created." << std::endl;}
    }
    if ((Constants::verbose == true)&& (s_bool_RatedAbbrExist == true)){std::cout << "Step 8. MM.DB and artist.adj not recently updated. ratedabbr.txt not updated." << std::endl;}

    // 9. Determine if a playlist exists, and if not, determine if it was identified as being selected in the user's config:
    // Determine if cleaned (path-corrected) playlist selected (s_bool_PlaylistExist) cleanedplaylist.txt exists, doesFileExist (const std::string& name);
    // sets bool6 and bool7

    s_bool_PlaylistExist = doesFileExist (Constants::cleanedPlaylist);
    if (s_bool_PlaylistExist == true) {
        s_bool_PlaylistSelected = true;
        if (Constants::verbose == true){std::cout << "Step 9. Playlist exists and was not updated." << std::endl;}
    }
    //a. If s_bool_PlaylistExist is false, determine if playlist was identified as selected in user config (sets s_bool_PlaylistSelected)
    if (s_bool_PlaylistExist == false){
        if (Constants::verbose == true){std::cout << "Step 9. Playlist not found. Checking user config for playlist selection." << std::endl;}
        //getConfigEntry: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
        std::string s_selectedplaylist = userconfig::getConfigEntry(7);
        if (s_selectedplaylist != "") {
            s_bool_PlaylistSelected = true;
            if (Constants::verbose == true){std::cout << "Step 9. Playlist found in user config." << std::endl;}
        }
    }
    // 10. If a playlist was identified in the user config, but the playlist file is not found, obtain the playlist file: If user configuration
    // exists, MM4 data exists, songs table exists (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist are all true), and playlist from user config exists (s_bool_PlaylistSelected is true),
    // but cleaned playlist does not (s_bool_PlaylistExist is false), run function to obtain cleaned playlist file getPlaylist() then set s_bool_PlaylistExist to true,
    // rechecking, run doesFileExist (const std::string& name) function. Evaluates s_bool_PlaylistExist and sets to true (after running getPlaylist) if
    // initially false

    if ((s_bool_IsUserConfigSet == true) && (s_bool_MMdbExist == true) && (s_bool_CleanLibExist == true) && (s_bool_PlaylistSelected == true) && (s_bool_PlaylistExist == false))
    {
        if (Constants::verbose == true){std::cout << "Step 10. Playlist missing, but was found in user config. Recreating playlist" << std::endl;}
        getPlaylist();
        s_bool_PlaylistExist = doesFileExist (Constants::cleanedPlaylist);
        if (s_bool_PlaylistExist == false) {std::cout << "Step 10. Something went wrong at the function getPlaylist." << std::endl;}
    }
    if ((Constants::verbose == true)&& (s_bool_PlaylistExist == true)){std::cout << "Step 10. Playlist exists and was not updated." << std::endl;}

    // NOTE: functions used in the next  three steps (11-13) will later be reused when adding tracks to
    // playlist - here, this is to get the initial values if a playlist exists

    //11. If playlist exists, calculate the playlist size: If cleaned playlist exists (s_bool_PlaylistExist is true), obtain playlist size
    // using function cstyleStringCount(),  s_playlistSize = cstyleStringCount(cleanedPlaylist);

    if (s_bool_PlaylistExist == true) {
        s_playlistSize = cstyleStringCount(Constants::cleanedPlaylist);
        if (Constants::verbose == true){std::cout << "Step 11. Playlist size is: "<< s_playlistSize << std::endl;}
    }

    // 12. If playlist exists, obtain the historical count (in addition to the playlist count) up to the sequential track limit:
    // If cleaned playlist exists (s_bool_PlaylistExist is true), obtain the historical count (in addition to the playlist count) up to the
    // sequential track limit. A variable is needed (which later will be used to obtain additional play history outside of
    // playlist, as part of a later function to make a new track selection), using the variable s_histCount. The value is
    // calculated [ can be modified to use the function  to added function void getHistCount(&s_SequentialTrackLimit,&s_playlistSize),
    // or just: s_histCount = long(s_SequentialTrackLimit) – long(s_playlistSize); this uses both playlist size from 10
    // and SequentialTrackLimit obtained with data from function getDBStats()]
    if (s_bool_PlaylistExist == true) {
        s_playlistSize = cstyleStringCount(Constants::cleanedPlaylist);
        s_histCount = int(s_SequentialTrackLimit - s_playlistSize);
        if (Constants::verbose == true){std::cout << "Step 12. s_histCount is: "<< s_histCount << std::endl;}
    }

    //13. If playlist exists, artist statistics are processed, and modified database exists, create/update excluded artists
    // list: If cleaned playlist exists (s_bool_PlaylistExist is true), and artistsadj.txt exists (s_bool_artistsadjExist is true) and modified database exists
    // (s_bool_RatedAbbrExist), run function getExcludedArtists() to create/update excluded artists list using vectors
    // read in from the following files: cleanlib.dsv, artistsadj.txt, and cleanedplaylist.txt. Writes artistexcludes.txt. Also,
    // creates temporary database (ratedabbr2.txt) with playlist position numbers for use in subsequent functions,
    //ratingCodeSelected and selectTrack

    if (s_bool_PlaylistExist == true)   {
        getExcludedArtists(s_histCount, s_playlistSize);
    }

    ui->currentplsizeLabel->setText(tr("Current playlist size: ") + QString::number(s_playlistSize));
    //playlistdaysLabel
    ui->playlistdaysLabel->setText(tr("Current playlist days (based on est. listening rate): ") + QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
    //s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong)
    ui->repeatFreq1SpinBox->setValue(m_prefs.repeatFreqCode1);
    ui->addtrksspinBox->setValue(m_prefs.tracksToAdd);
    ui->statusBar->addPermanentWidget(ui->progressBarPL);
    ui->progressBarPL->hide();
    ui->newtracksqtyLabel->setText(tr("New tracks qty: ") + QString::number(s_rCode1TotTrackQty));
    ui->factor3horizontalSlider->setMinimum(10);
    ui->factor3horizontalSlider->setMaximum(120);
    ui->factor3horizontalSlider->setValue(int(s_daysTillRepeatCode3));
    ui->factor3IntTxtLabel->setNum(s_daysTillRepeatCode3);
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor4doubleSpinBox->setValue(m_prefs.s_repeatFactorCode4);
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor5doubleSpinBox->setValue(m_prefs.s_repeatFactorCode5);
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor6doubleSpinBox->setValue(m_prefs.s_repeatFactorCode6);
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor7doubleSpinBox->setValue(m_prefs.s_repeatFactorCode7);
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor8doubleSpinBox->setValue(m_prefs.s_repeatFactorCode8);

    ui->monthsradioButton->click();
}


void ArchSimian::on_addsongsButton_clicked(){
    int numTracks = ui->addtrksspinBox->value();
    ui->statusBar->showMessage("Adding " + QString::number(numTracks) + " tracks to playlist",10000);
    //ui->progressBarPL->show();
    remove("songtext.txt");
    std::ofstream songtext("songtext.txt",std::ios::app); // output file for writing final song selection data for ui display
    int n;
    s_ratingNextTrack = ratingCodeSelected(s_ratingRatio3,s_ratingRatio4,s_ratingRatio5,s_ratingRatio6,
                                           s_ratingRatio7,s_ratingRatio8, s_rCode1TotTrackQty, m_prefs.repeatFreqCode1);
    for (n=0; n < numTracks; n++){
        if (Constants::verbose == true) std::cout << "Rating for the next track is " << s_ratingNextTrack << std::endl;
        selectTrack(s_ratingNextTrack,&s_selectedTrackPath);
        std::string shortselectedTrackPath;
        shortselectedTrackPath = s_selectedTrackPath;
        std::string key1 ("/");
        std::string key2 ("_");
        //std::string key3 (".mp3");
        shortselectedTrackPath.erase(0,12);
        std::size_t found = shortselectedTrackPath.rfind(key1);
        std::size_t found1 = shortselectedTrackPath.rfind(key2);
        //std::size_t found2 = shortselectedTrackPath.rfind(key3);
        if (found!=std::string::npos){shortselectedTrackPath.replace (found,key1.length(),", ");}
        if (found1!=std::string::npos){shortselectedTrackPath.replace (found,key2.length()," ");}
        //if (found2!=std::string::npos){shortselectedTrackPath.replace (found,key3.length()," ");}
        //std::cout <<s_playlistSize<< ". " << shortselectedTrackPath<<std::endl;
        s_playlistSize = cstyleStringCount("cleanedplaylist.txt");
        songtext << s_playlistSize<<". "<< shortselectedTrackPath <<'\n';
        //if (Constants::verbose == true) std::cout <<'\n';
        //std::cout << ", rating " << "___" << std::endl;
        if (Constants::verbose == true) std::cout << "Playlist length is: " << s_playlistSize << " tracks." << std::endl;
        s_histCount = long(s_SequentialTrackLimit) - long(s_playlistSize);
        getExcludedArtists(s_histCount, s_playlistSize);
        s_ratingNextTrack = ratingCodeSelected(s_ratingRatio3,s_ratingRatio4,s_ratingRatio5,s_ratingRatio6,
                                               s_ratingRatio7,s_ratingRatio8, s_rCode1TotTrackQty, m_prefs.repeatFreqCode1);
        ui->currentplsizeLabel->setText(tr("Current playlist size: ") + QString::number(s_playlistSize));
        ui->playlistdaysLabel->setText(tr("Current playlist days (based on est. listening rate): ") + QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
    }
    songtext.close();
    //ui->progressBarPL->hide();
    ui->statusBar->showMessage("Added " + QString::number(numTracks) + " tracks to playlist",50000);
    QFile songtext1("songtext.txt");
    if(!songtext1.open(QIODevice::ReadOnly))
        QMessageBox::information(nullptr,"info",songtext1.errorString());
    QTextStream in(&songtext1);
    ui->songsaddtextBrowser->setText(in.readAll());

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
    s_musiclibrarydirname= QFileDialog::getExistingDirectory(
                this,
                tr("Select Shared Music Library Directory"),
                "/"
                );
    ui->setlibrarylabel->setText(QString(s_musiclibrarydirname));
    // Write description note and directory configuration to archsimian.conf
    m_prefs.musicLibraryDir = s_musiclibrarydirname;

    //std::ofstream userconfig(Constants::userFileName);
    //std::string str("# Location of music library");
    //userconfig << str << "\n";  // Write to line 1, archsimian.conf
    //str = s_musiclibrarydirname.toStdString();
    //userconfig << str << "\n";  // Write to line 2, archsimian.conf
    //userconfig.close();
    // dim the setlibraryButton button
    //ui->setlibraryButton->setEnabled(false);
    // Enable the reset button
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
    const QString mmbackuppldirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select MediaMonkey Playlist Backup Directory"),
                "/"
                );
    ui->setmmpllabel->setText(QString(mmbackuppldirname));
    m_prefs.mmPlaylistDir = mmbackuppldirname;
    // Write description note and directory configuration to archsimian.conf
    //std::ofstream userconfig(Constants::userFileName, std::ios::app);
    //std::string str("# Location of MediaMonkey Playlist Backup Directory");
    //userconfig << str << "\n";  // Write to line 3, archsimian.conf
    //str = s_mmbackuppldirname.toStdString();
    //userconfig << str << "\n"; // Write to line 4, archsimian.conf
    //userconfig.close();
    //dim the setmmplButton button
    //ui->setmmplButton->setEnabled(false); // test with enabled
    //enable the reset button
    // Activate the last of three config buttons
    //ui->setmmdbButton->setEnabled(true);
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
    const QString mmbackupdbdirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select MediaMonkey Database (MM.DB) Backup Directory"),
                "/"
                );
    ui->setmmdblabel->setText(QString(mmbackupdbdirname));
    m_prefs.mmBackupDBDir = mmbackupdbdirname;
    // Write description note and directory configuration to archsimian.conf
    //std::ofstream userconfig(Constants::userFileName, std::ios::app);
    //std::string str("# Location of MediaMonkey Database Backup Directory");
    //userconfig << str << "\n";  // Write to line 5, archsimian.conf
    //str = s_mmbackupdbdirname.toStdString();
    //userconfig << str << "\n"; // Write to line 6, archsimian.conf
    //userconfig.close();
    //dim the setmmdbButton button
    //ui->setmmdbButton->setEnabled(true);
    //enable the reset button
}



// User selects playlist from configured directory for 'backup playlists'
void ArchSimian::on_getplaylistButton_clicked()
{
    QFileDialog setgetplaylistdialog;
    QString selectedplaylist = QFileDialog::getOpenFileName (
                this,
                "Select playlist for which you will add tracks",
                QString(m_prefs.defaultPlaylist),//default dir for playlists
                "playlists(.m3u) (*.m3u)");
    m_prefs.defaultPlaylist = selectedplaylist;
    ui->setgetplaylistLabel->setText("Selected: " + QString(selectedplaylist)); // to redo the config file to use QSettings instead
    // Write description note and playlist name to archsimian.conf
    //std::ofstream userconfig(Constants::userFileName, std::ios::app);
    //std::string str("# Name of default playlist");
    //userconfig << str << "\n";  // Write to line 7, archsimian.conf
    //str = selectedplaylist.toStdString();
    //userconfig << str << "\n"; // Write to line 8, archsimian.conf
    //userconfig.close();
}

void ArchSimian::on_mainQTabWidget_tabBarClicked(int index)
{
    if (index == 2) // if the Statistics tab is selected, refresh stats
    {
        ;
        ui->ybrLabel3->setText("Years between repeats for rating code 3 (5 stars): " + QString::number(s_yrsTillRepeatCode3,'g', 3) +
                               " (in days: " + QString::fromStdString(std::to_string(int(s_daysTillRepeatCode3))) + ")");
        ui->ybrLabel4->setText("Years between repeats for rating code 4 (4 stars): " + QString::number(s_yrsTillRepeatCode4,'g', 3));
        ui->ybrLabel5->setText("Years between repeats for rating code 5 (3 1/2 stars): " + QString::number(s_yrsTillRepeatCode5,'g', 3));
        ui->ybrLabel6->setText("Years between repeats for rating code 6 (3 stars): " + QString::number(s_yrsTillRepeatCode6,'g', 3));
        ui->ybrLabel7->setText("Years between repeats for rating code 7 (2 1/2 stars): " + QString::number(s_yrsTillRepeatCode7,'g', 3));
        ui->ybrLabel8->setText("Years between repeats for rating code 8 (2 stars): " + QString::number(s_yrsTillRepeatCode8,'g', 3));

        ui->totadjtracksLabel->setText(tr("Current playlist size is: ") + QString::fromStdString(std::to_string(s_playlistSize)));
        ui->tottracksLabel->setText("Total tracks in the library is: " + QString::fromStdString(std::to_string(s_totalLibQty)));
        ui->totratedtracksLabel->setText("Total rated tracks in the library is: " + QString::fromStdString(std::to_string(s_totalRatedQty)));
        ui->totratedtimeLabel->setText("Total rated time (in hours) is: " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
        ui->dailylistenLabel->setText("Calculated daily listening rate (in hours) is: " + QString::number(s_listeningRate,'g', 3));

    }
}

//void ArchSimian::on_pushButton_clicked()
//{

//}

//void ArchSimian::on_refreshdbButton_clicked()
//{

    // Add code to 'goto' beginning of this cpp, then delete the rest of the below code except last few lines?


//    s_LastTableDate = getLastTableDate(); // function is in dependents.cpp
//   ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(s_MMdbDate)+ tr(" Library date: ")+ QString::fromStdString(s_LastTableDate));
    //ui->updatestatusLabel->setText("Library update completed.");
//}

void ArchSimian::on_addtrksspinBox_valueChanged(int s_numTracks)
{
        m_prefs.tracksToAdd = s_numTracks;
        //s_listeningRate //double(s_AvgMinsPerSong*value)/s_avgListeningRateInMins)
        ui->daystoaddLabel->setText(tr("Based on a daily listening rate (in mins.) of ") + QString::number(s_avgListeningRateInMins,'g', 3)
                                    + tr(", tracks per day is ") + QString::number((s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3)+tr(", so"));
        ui->daystracksLabel->setText(tr("days added for 'Add Songs' quantity selected above will be: ") + QString::number((m_prefs.tracksToAdd * s_AvgMinsPerSong)/s_avgListeningRateInMins,'g', 3));

        //days added for 'Add Songs' quantity selected above will be:

}

void ArchSimian::on_repeatFreq1SpinBox_valueChanged(int myvalue)
{
    m_prefs.repeatFreqCode1 = myvalue;
    //s_repeatFreqForCode1 = ui->repeatFreq1SpinBox->value();
}

void ArchSimian::loadSettings()
{
    QSettings settings;
    m_prefs.repeatFreqCode1 = settings.value("repeatFreqCode1", 20).toInt();
    m_prefs.tracksToAdd = settings.value("tracksToAdd", 50).toInt();
    m_prefs.defaultPlaylist = settings.value("defaultPlaylist", "").toString();
    m_prefs.musicLibraryDir = settings.value("musicLibraryDir", "").toString();
    m_prefs.mmBackupDBDir = settings.value("mmBackupDBDir", "").toString();
    m_prefs.mmPlaylistDir = settings.value("mmPlaylistDir", "").toString();
    m_prefs.s_daysTillRepeatCode3 = settings.value("s_daysTillRepeatCode3", 65).toDouble();
    m_prefs.s_repeatFactorCode4 = settings.value("s_repeatFactorCode4", 2.7).toDouble();
    m_prefs.s_repeatFactorCode5 = settings.value("s_repeatFactorCode5", 2.1).toDouble();
    m_prefs.s_repeatFactorCode6 = settings.value("s_repeatFactorCode6", 2.2).toDouble();
    m_prefs.s_repeatFactorCode7 = settings.value("s_repeatFactorCode7", 1.6).toDouble();
    m_prefs.s_repeatFactorCode8 = settings.value("s_repeatFactorCode8", 1.4).toDouble();
    s_mmBackupDBDir = m_prefs.mmBackupDBDir;
}

void ArchSimian::saveSettings()
{
    QSettings settings;
    settings.setValue("repeatFreqCode1", m_prefs.repeatFreqCode1);
    settings.setValue("tracksToAdd", m_prefs.tracksToAdd);
    settings.setValue("defaultPlaylist",m_prefs.defaultPlaylist);
    settings.setValue("musicLibraryDir",m_prefs.musicLibraryDir);
    settings.setValue("mmBackupDBDir",m_prefs.mmBackupDBDir);
    settings.setValue("mmPlaylistDir",m_prefs.mmPlaylistDir);
    settings.setValue("s_daysTillRepeatCode3",m_prefs.s_daysTillRepeatCode3);
    settings.setValue("s_repeatFactorCode4",m_prefs.s_repeatFactorCode4);
    settings.setValue("s_repeatFactorCode5",m_prefs.s_repeatFactorCode5);
    settings.setValue("s_repeatFactorCode6",m_prefs.s_repeatFactorCode6);
    settings.setValue("s_repeatFactorCode7",m_prefs.s_repeatFactorCode7);
    settings.setValue("s_repeatFactorCode8",m_prefs.s_repeatFactorCode8);

}
void ArchSimian::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void ArchSimian::on_daysradioButton_clicked()
{
    s_dateTranslation = 365;
    dateTransTextVal = " days";
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
}

void ArchSimian::on_weeksradioButton_clicked()
{
    s_dateTranslation = 52;
    dateTransTextVal = " weeks";
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
}

void ArchSimian::on_monthsradioButton_clicked()
{
    s_dateTranslation = 12;
    dateTransTextVal = " months";
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
}

void ArchSimian::on_yearsradioButton_clicked()
{
    s_dateTranslation = 1;
    dateTransTextVal = " years";
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
  }

void ArchSimian::on_factor3horizontalSlider_valueChanged(int value)
{
m_prefs.s_daysTillRepeatCode3 = value;
s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
s_daysTillRepeatCode3 = m_prefs.s_daysTillRepeatCode3;
sliderBaseVal3 = s_yrsTillRepeatCode3;
ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
s_repeatFactorCode5 = m_prefs.s_repeatFactorCode5;
s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
s_repeatFactorCode6 = m_prefs.s_repeatFactorCode6;
s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
s_repeatFactorCode7 = m_prefs.s_repeatFactorCode7;
s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
s_repeatFactorCode8 = m_prefs.s_repeatFactorCode8;
}

void ArchSimian::on_factor4doubleSpinBox_valueChanged(double argfact4)
{
    m_prefs.s_repeatFactorCode4 = argfact4;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(argfact4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode5 = m_prefs.s_repeatFactorCode5;
    s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode6 = m_prefs.s_repeatFactorCode6;
    s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode7 = m_prefs.s_repeatFactorCode7;
    s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode8 = m_prefs.s_repeatFactorCode8;
}

void ArchSimian::on_factor5doubleSpinBox_valueChanged(double argfact5)
{
    m_prefs.s_repeatFactorCode5 = argfact5;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    ui->factor5label->setText(QString::number(argfact5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode5 = m_prefs.s_repeatFactorCode5;
    s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode6 = m_prefs.s_repeatFactorCode6;
    s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode7 = m_prefs.s_repeatFactorCode7;
    s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode8 = m_prefs.s_repeatFactorCode8;
}

void ArchSimian::on_factor6doubleSpinBox_valueChanged(double argfact6)
{
    m_prefs.s_repeatFactorCode6 = argfact6;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode5 = m_prefs.s_repeatFactorCode5;
    s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
    ui->factor6label->setText(QString::number(argfact6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode6 = m_prefs.s_repeatFactorCode6;
    s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode7 = m_prefs.s_repeatFactorCode7;
    s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
}

void ArchSimian::on_factor7doubleSpinBox_valueChanged(double argfact7)
{
    m_prefs.s_repeatFactorCode7 = argfact7;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode5 = m_prefs.s_repeatFactorCode5;
    s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode6 = m_prefs.s_repeatFactorCode6;
    s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
    ui->factor7label->setText(QString::number(argfact7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode7 = m_prefs.s_repeatFactorCode7;
    s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
}

void ArchSimian::on_factor8doubleSpinBox_valueChanged(double argfact8)
{
    m_prefs.s_repeatFactorCode8 = argfact8;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode5 = m_prefs.s_repeatFactorCode5;
    s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode6 = m_prefs.s_repeatFactorCode6;
    s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode7 = m_prefs.s_repeatFactorCode7;
    s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
    ui->factor8label->setText(QString::number(argfact8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
}


