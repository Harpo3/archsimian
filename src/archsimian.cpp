//archsimian.cpp is equivalent to MainWindow.cpp. This and archsimian.h are
// where the UI code resides. The main () function is located at main.cpp

#include <QSettings>
#include <QtWidgets>
#include <QPalette>
#include <QStandardPaths>
#include <QMessageBox>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "archsimian.h"
#include "constants.h"
#include "ui_archsimian.h"
#include "dependents.h"
#include "getartistexcludes.h"
#include "getplaylist.h"
#include "writesqlfile.h"
#include "basiclibfunctions.h"
#include "playlistfunctions.h"
#include "buildalbumexcllibrary.h"
#include "getartistadjustedcount.h"
#include "code1.h"
#include "albumidandselect.h"

template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N]) {//Function to execute command line with parameters
    assert((N > 0) && (argv[N - 1] == nullptr));
    return execvp(file, const_cast<char* const*>(argv));
}

inline bool doesFileExist (const std::string& name) {
    struct stat buffer{};
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
static bool s_includeNewTracks{true};
static bool s_includeAlbumVariety{true};
static bool s_noAutoSave{true};
static bool s_disableNotificationAddTracks{false};
static int s_uniqueCode1ArtistCount{0};
static int s_code1PlaylistCount{0};
static int s_lowestCode1Pos{99999};
static std::string s_artistLastCode1{""};
static std::string s_selectedCode1Path{""};
static std::string s_selectedTrackPath{""};

// Repeat factor codes used to calculate repeat rate in years
static double s_SequentialTrackLimit = 0;
static double s_daysTillRepeatCode3 = Constants::kUserDefaultDaysTillRepeatCode3;
static double s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
static double s_repeatFactorCode4 = Constants::kUserDefaultRepeatFactorCode4;
static double s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
static double s_repeatFactorCode5 = Constants::kUserDefaultRepeatFactorCode5;
static double s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
static double s_repeatFactorCode6 = Constants::kUserDefaultRepeatFactorCode6;
static double s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
static double s_repeatFactorCode7 = Constants::kUserDefaultRepeatFactorCode7;
static double s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
static double s_repeatFactorCode8 = Constants::kUserDefaultRepeatFactorCode8;
static double s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
static int s_rCode0TotTime{0};
static int s_rCode1TotTime{0};
static int s_rCode3TotTime{0};
static int s_rCode4TotTime{0};
static int s_rCode5TotTime{0};
static int s_rCode6TotTime{0};
static int s_rCode7TotTime{0};
static int s_rCode8TotTime{0};
// Variables declared for use with artist calculations
static double s_yrsTillRepeatCode3factor = 1 / s_yrsTillRepeatCode3;
static double s_yrsTillRepeatCode4factor = 1 / s_yrsTillRepeatCode4;
static double s_yrsTillRepeatCode5factor = 1 / s_yrsTillRepeatCode5;
static double s_yrsTillRepeatCode6factor = 1 / s_yrsTillRepeatCode6;
static double s_yrsTillRepeatCode7factor = 1 / s_yrsTillRepeatCode7;
static double s_yrsTillRepeatCode8factor = 1 / s_yrsTillRepeatCode8;
// Variables declared for use with album-level variety calculations
static int s_minalbums;
static int s_mintrackseach;
static int s_mintracks;
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
static double s_totAdjHours{0.0};
static double s_adjHoursCode3{0.0};
static double s_adjHoursCode4{0.0};
static double s_adjHoursCode5{0.0};
static double s_adjHoursCode6{0.0};
static double s_adjHoursCode7{0.0};
static double s_adjHoursCode8{0.0};
static double s_DaysBeforeRepeatCode3{0.0};
static double s_totHrsLast60Days{0.0};
static double s_totalAdjRatedQty{0.0};
static int s_playlistSize{0};
static int s_ratingNextTrack{0};
//static std::string s_MMdbDate{""};
//static std::string s_LastTableDate{""};
static long s_histCount{0};
static double s_AvgMinsPerSong{0.0};
static double s_avgListeningRateInMins{0.0};
static int s_repeatFreqForCode1{0};
static int s_dateTranslation{0};
static QString dateTransTextVal{" months"};
static double sliderBaseVal3{0.0};
static QString s_mmBackupDBDir{""};
static QString s_musiclibrarydirname{""};
static QString s_mmPlaylistDir{""};
static QString s_defaultPlaylist{""};
static QString s_winDriveLtr;
//QApplication::applicationName();
static QString appDataPathstr = QDir::homePath() + "/.local/share/archsimian";
static QDir appDataPath = appDataPathstr;
static std::string cleanLibFile = appDataPathstr.toStdString()+"/cleanlib.dsv";


ArchSimian::ArchSimian(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ArchSimian)
{   
    QWidget setupwindow;
    m_sSettingsFile = QApplication::applicationDirPath().left(1) + ":/archsimian.conf"; // sets the config file location for QSettings
    loadSettings(); // QSettings: load user settings from archsimian.conf file
    s_daysTillRepeatCode3 = m_prefs.s_daysTillRepeatCode3;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
    s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    s_repeatFactorCode5 = m_prefs.s_repeatFactorCode5;
    s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
    s_repeatFactorCode6 = m_prefs.s_repeatFactorCode6;
    s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
    s_repeatFactorCode7 = m_prefs.s_repeatFactorCode7;
    s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
    s_repeatFactorCode8 = m_prefs.s_repeatFactorCode8;
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    sliderBaseVal3 = m_prefs.s_daysTillRepeatCode3 / Constants::kDaysInYear;
    s_mmBackupDBDir = m_prefs.mmBackupDBDir;
    s_musiclibrarydirname = m_prefs.musicLibraryDir;
    s_defaultPlaylist = m_prefs.defaultPlaylist;
    s_mmPlaylistDir = m_prefs.mmPlaylistDir;
    s_includeNewTracks = m_prefs.s_includeNewTracks;
    s_includeAlbumVariety = m_prefs.s_includeAlbumVariety;
    s_noAutoSave = m_prefs.s_noAutoSave;
    s_disableNotificationAddTracks = m_prefs.s_disableNotificationAddTracks;
    s_minalbums = m_prefs.s_minalbums;
    s_mintrackseach= m_prefs.s_mintrackseach;
    s_mintracks = m_prefs.s_mintracks;
    s_repeatFreqForCode1 = m_prefs.repeatFreqCode1;
    getWindowsDriveLtr(s_defaultPlaylist, &s_winDriveLtr);
    m_prefs.s_WindowsDriveLetter = s_winDriveLtr;

    ui->setupUi(this);

    ui->mainQTabWidget->setCurrentIndex(0);

    // Step 1. Determine if user configuration exists:
    //
    // Use QSettings archsimian.conf file entries to check entries for mmBackupDBDir, mmPlaylistDir, and musicLibraryDir
    // If any are blank, or the dir found does not exist, or file MM.DB does not exist at the location specified, then
    // launch child window for user to set the locations prior to launch of main window. When child window
    // is closed verfiy the locations anf files selected exist.
    //
    // UI configuration: determine state of user config
    //
    if ((s_mmBackupDBDir != nullptr) && (s_musiclibrarydirname != nullptr) && (s_mmPlaylistDir != nullptr)){
        // add code to verify file /dir locations *****************
        if (Constants::kVerbose) {std::cout << "Step 1. s_mmBackupDBDir, s_musiclibrarydirname & mmPlaylistDirset up." <<std::endl;}
        s_bool_IsUserConfigSet = true;
    }
    else {
        s_bool_IsUserConfigSet = false;
        std::cout << "Step 1. Unable to open archsimian.conf configuration file or file has no data. s_bool_IsUserConfigSet result: "<< s_bool_IsUserConfigSet<<std::endl;
    }
    //If configuration has already been set, populate the ui labels accordingly
    if (s_bool_IsUserConfigSet)
    {
        if (Constants::kVerbose) std::cout << "Step 1. Configuration has already been set. s_bool_IsUserConfigSet result: "<<s_bool_IsUserConfigSet<<std::endl;
        m_prefs.musicLibraryDir = s_musiclibrarydirname;
        if (s_includeNewTracks) {ui->InclNewcheckbox->setChecked(true);}
        if (s_includeAlbumVariety) {
            ui->albumscheckBox->setChecked(true);
            ui->mainQTabWidget->setTabEnabled(4, true);
        }
        ui->menuBar->setDisabled(false);
        ui->setlibrarylabel->setText(QString(s_musiclibrarydirname));
        ui->setlibraryButton->setEnabled(true);
        ui->setmmpllabel->setText(QString(s_defaultPlaylist));
        ui->setmmplButton->setEnabled(true);
        ui->setmmdblabel->setText(s_mmBackupDBDir);
        ui->setmmpllabel->setText(s_mmPlaylistDir);
        ui->setgetplaylistLabel->setText("Selected: " + s_defaultPlaylist);
        ui->instructionlabel->setText(tr(""));
        if (s_defaultPlaylist == ""){
            ui->setgetplaylistLabel->setText("  ****** No playlist has been selected ******");
            ui->addsongsButton->setEnabled(false);
            ui->exportplaylistButton->setEnabled(false);
        }
        ui->setmmdbButton->setEnabled(true);
        bool needUpdate = recentlyUpdated(s_mmBackupDBDir);
        //bool needUpdate = getMMdbDate(); // function getMMdbDate() is from dependents.cpp
        if (Constants::kVerbose) std::cout << "Step 1. Checking getMMdbDate(): "<<needUpdate<<std::endl;
        if (needUpdate == 0)
        {
            std::string MMdbDate = getMMdbDate(s_mmBackupDBDir);
            std::string LastTableDate = getLastTableDate();
            ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(MMdbDate)+
                                           tr(", Library date: ")+ QString::fromStdString(LastTableDate));
        }
    }
    else {  // Otherwise, configuration has not been set. Load instructions for user to locate and set config        
        s_bool_IsUserConfigSet = false;
        if (Constants::kVerbose) std::cout << "Step 1. Configuration has not been set. Adjusting gui settings"<<std::endl;
        ui->setlibrarylabel->setText(tr(""));
        ui->setlibraryButton->setEnabled(true);
        ui->setmmpllabel->setText(tr(""));
        ui->setmmplButton->setEnabled(true);
        ui->setmmdblabel->setText(tr(""));
        ui->setmmdbButton->setEnabled(true);
        ui->menuBar->setDisabled(true);
        ui->autosavecheckBox->setChecked(true);
        ui->setgetplaylistLabel->setText(tr("Select playlist for adding tracks"));
        ui->mainQTabWidget->setCurrentIndex(1);
        ui->setlibrarylabel->setText(tr("Set the home directory (top level) of the music library and store in user settings."));
        ui->setmmpllabel->setText(tr("Select the shared Windows directory where you store your m3u playlists."));
        ui->setmmdblabel->setText(tr("Select the shared Windows directory where you stored the MediaMonkey database (MM.DB) backup file."));
        ui->instructionlabel->setText(tr("ArchSimian Setup: \n(1) Identify the location where your music library is stored  \n(2) Set the "
                                         "location where you did backups for your M3U playlist \n(3) Set the location where you did backup "
                                         "of the MM.DB file \n(4) Check whether to enable new tracks \n(5) Check whether to enable album-level "
                                         "variety. \n(6) Restart the program."));
    }

    // Step 2. Determine if MM.DB database file exists: Run doesFileExist (const std::string& name) function (sets s_bool_MMdbExist).
    if (s_bool_IsUserConfigSet) {
        std::string mmdbdir = s_mmBackupDBDir.toStdString();
        const std::string mmpath = mmdbdir + "/MM.DB";
        s_bool_MMdbExist = doesFileExist(mmpath);

        //    a. If s_boolIsUserConfigSet is true, but s_bool_MMdbExist is false, report to user that MM.DB was not found at the
        //       location specified and set s_bool_IsUserConfigSet to false
        //    b. If s_boolIsUserConfigSet is false, set s_bool_MMdbExist to false

        if (Constants::kVerbose) std::cout << "Step 2. Does MM.DB file exist. s_bool_MMdbExist result: "<< s_bool_MMdbExist << std::endl;
    }
    if (!s_bool_IsUserConfigSet) {
        s_bool_MMdbExist = false;
        s_bool_MMdbUpdated = false;
        s_bool_PlaylistExist = false;
    }
    if ((s_bool_IsUserConfigSet) && (!s_bool_MMdbExist)) {
        if (Constants::kVerbose) std::cout << "Step 2. MM.DB was not found at the location you specified. Setting s_bool_IsUserConfigSet to false." << std::endl;
        s_bool_IsUserConfigSet = false;
    }

    // Step 3. Determine if Archsimian songs table exists: If user configuration exists and MM4 data exists (s_bool_IsUserConfigSet and s_bool_MMdbExist are true),
    //determine if cleanlib.dsv songs table (Constants::cleanLibFile) exists in AS, function doesFileExist (const std::string& name)  (dir paths corrected,
    // imported from MM.DB) (sets s_bool_CleanLibExist)

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist)) {
        bool tmpbool{true};
        tmpbool = doesFileExist(cleanLibFile);
        if (Constants::kVerbose) std::cout << "Step 3. tmpbool indicating cleanLibFile file: " << cleanLibFile <<"  exists result: " <<tmpbool  << std::endl;
        if (tmpbool){ // check that file is not empty
            //Check whether the songs table currently has any data in it
            std::streampos cleanLibFilesize;
            char * memblock;
            std:: ifstream file (cleanLibFile, std::ios::in|std::ios::binary|std::ios::ate);
            if (file.is_open())
            {
                cleanLibFilesize = file.tellg();
                if (Constants::kVerbose) std::cout << "Step 3. cleanLibFilesize result: " <<cleanLibFilesize  << std::endl;
                memblock = new char [static_cast<unsigned long>(cleanLibFilesize)];
                file.seekg (0, std::ios::beg);
                file.read (memblock, cleanLibFilesize);
                file.close();
                delete[] memblock;
            }
            if (cleanLibFilesize != 0) {s_bool_CleanLibExist = true;}
        }
    }
    if ((Constants::kVerbose)&&(s_bool_IsUserConfigSet)) std::cout << "Step 3. Does CleanLib file exist. s_bool_CleanLibExist result: "
                                                                                  << s_bool_CleanLibExist << std::endl;

    // 4. Determine if MM.DB was recently updated: s_bool_MMdbUpdated is set by comparing MM.DB file date
    // to CleanLib (songs table) file date. If the MM.DB file date is newer (greater) than the CleanLib file date
    // will need to be updated.

    if (s_bool_IsUserConfigSet) {
        s_bool_MMdbUpdated = recentlyUpdated(s_mmBackupDBDir);
        if (Constants::kVerbose) std::cout << "Step 4. Is the MM.DB file date newer (greater) than the CleanLib file date."
                                                     " s_bool_MMdbUpdated result: "<< s_bool_MMdbUpdated << std::endl;
        // set ui labels if MM.DB was recently updated
        if (s_bool_MMdbUpdated) // bool s_bool_MMdbUpdated: 1 means refresh DB, 0 means skip
            // If result is 1, removeAppData ratedabbr.txt, ratedabbr2.txt, artistsadj.txt, playlistposlist.txt, artistexcludes.txt, and cleanedplaylist.txt files
        {
            QMessageBox msgBox;
            msgBox.setText("ArchSimian Database Update: A new MediaMonkey database backup has been "
                                                                 "identified. Updating ArchSimian."
                                                                 " The program window will appear when completed.");
            msgBox.exec();
            //KDEmessage("ArchSimian Library Update Notification","A new MediaMonkey database backup was "
              //                                                  "identified...updating the ArchSimian "
                //                                                "database. The program will launch when completed...",20);
            removeAppData("ratedabbr.txt");
            s_bool_RatedAbbrExist = false;
            removeAppData("ratedabbr2.txt");
            removeAppData("artistsadj.txt");
            s_bool_artistsadjExist = false;
            removeAppData("playlistposlist.txt");
            removeAppData("artistexcludes.txt");
            s_bool_ExcludedArtistsProcessed = false;
            removeAppData("cleanedplaylist.txt");
            s_bool_PlaylistExist = false;
            ui->updatestatusLabel->setText(tr("MM.DB was recently backed up. Library has been rebuilt."));
        }
    }

    // Step 5. If user configuration and MM4 data exist, but the songs table does not, import songs table into Archsimian: If user configuration
    // and MM4 data exist, but the songs table does not (bool_IsUserConfigSet, s_bool_MMdbExist are true, s_bool_CleanLibExist is false), import songs table into AS, by running
    // writeSQLFile() function, which creates the temporary basic table file libtable.dsv; then run the getLibrary() function, which creates
    // the refined table file cleanlib.dsv The getLibrary() function completes the
    // following refinements: (a) corrects the directory paths to Linux, (b) adds random lastplayed dates for rated or "new-need-to-be-rated"
    // tracks that have no play history, (c) creates rating codes for any blank values found in GroupDesc col, using POPM values,
    // and (d) creates Artist codes (using col 1) and places the code in Custom2 if Custom2 is blank. Then set s_bool_CleanLibExist to true, rechecking,
    // run doesFileExist (const std::string& name) function. After verifying  cleanlib.dsv exists, remove temporary basic table file
    // libtable.dsv Evaluates s_bool_CleanLibExist for existence of cleanlib.dsv (cleanLibFile)
    if ((Constants::kVerbose)&&(!s_bool_MMdbUpdated)&&(s_bool_CleanLibExist)&&(s_bool_IsUserConfigSet)){
        std::cout << "Step 5. CleanLib file exists and MM.DB was not recently updated. Skip to Step 6."<< std::endl;}

    if (((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (!s_bool_CleanLibExist)) || (s_bool_MMdbUpdated)) {
        if (Constants::kVerbose) std::cout << "Step 5. User configuration and MM.DB exists, but the songs table does not, or MM.DB was"
                                                     "recently updated. Importing songs table (create CleanLib) into Archsimian from MM.DB..." <<std::endl;
        writeSQLFile();
        pid_t c_pid;// Create fork object; child to get database table into a dsv file, then child to open that table only
        // after it finishes getting written, not before.
        c_pid = fork(); // Run fork function
        int status; // For status of pid process
        if( c_pid == 0 ){ // Child process: Get songs table from MM4 database, and create libtable.dsv with table;
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
            removeSQLFile();
        }
    }
    s_bool_CleanLibExist = doesFileExist (cleanLibFile);
    if (s_bool_CleanLibExist) {removeAppData ("libtable.dsv");}
    else {
        std::cout << "Step 6. Unable to create cleanLibFile, cleanlib.dsv." << std::endl;
        s_bool_CleanLibExist = false;
    }

    // Step 6. If user configuration exists, MM.DB exists and songs table exists, process/update statistics: If user configuration exists, MM4 data exists,
    // songs table exists (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist are all true), run function to process/update statistics getDBStats()
    if ((Constants::kVerbose) && (s_bool_IsUserConfigSet)&& (s_bool_MMdbExist) && (s_bool_CleanLibExist))std::cout
            << "Step 6. User configuration exists, MM.DB exists and songs table exists. Processing database statistics:" << std::endl;
    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist)) {
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
        s_rCode0TotTime = (s_rCode0MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_rCode1TotTime = (s_rCode1MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_rCode3TotTime = (s_rCode3MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_rCode4TotTime = (s_rCode4MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_rCode5TotTime = (s_rCode5MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_rCode6TotTime = (s_rCode6MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_rCode7TotTime = (s_rCode7MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_rCode8TotTime = (s_rCode8MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        //  Total time listened in hours per rating code for each of six 10-day periods
        s_SQL10TotTimeListened = (s_SQL10TotTimeListened/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_SQL20TotTimeListened  = (s_SQL20TotTimeListened/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_SQL30TotTimeListened  = (s_SQL30TotTimeListened/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_SQL40TotTimeListened = (s_SQL40TotTimeListened/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_SQL50TotTimeListened = (s_SQL50TotTimeListened/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        s_SQL60TotTimeListened = (s_SQL60TotTimeListened/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor;
        // Compile statistics and declare additional statistical variables
        //Total number of tracks in the library
        s_totalLibQty = s_rCode0TotTrackQty + s_rCode1TotTrackQty + s_rCode3TotTrackQty + s_rCode4TotTrackQty +
                s_rCode5TotTrackQty + s_rCode6TotTrackQty + s_rCode7TotTrackQty + s_rCode8TotTrackQty;
        s_totalRatedQty = s_totalLibQty - s_rCode0TotTrackQty; //Total number of rated tracks in the library
        s_totHrsLast60Days = s_SQL10TotTimeListened + s_SQL20TotTimeListened + s_SQL30TotTimeListened + s_SQL40TotTimeListened
                + s_SQL50TotTimeListened + s_SQL60TotTimeListened; //Total listened hours in the last 60 days
        // User listening rate weighted avg calculated using the six 10-day periods, and applying sum-of-the-digits for weighting
        s_listeningRate = ((s_SQL10TotTimeListened/Constants::kDaysPerListeningPeriod)*0.3) + ((s_SQL20TotTimeListened/Constants::kDaysPerListeningPeriod)*0.25)
                + ((s_SQL30TotTimeListened/Constants::kDaysPerListeningPeriod)*0.2) + ((s_SQL40TotTimeListened/Constants::kDaysPerListeningPeriod)*0.15) +
                ((s_SQL50TotTimeListened/Constants::kDaysPerListeningPeriod)*0.1) + ((s_SQL60TotTimeListened/Constants::kDaysPerListeningPeriod)*0.05);
        s_adjHoursCode3 = (1 / s_yrsTillRepeatCode3) * s_rCode3TotTime;
        s_adjHoursCode4 = (1 / s_yrsTillRepeatCode4) * s_rCode4TotTime;
        s_adjHoursCode5 = (1 / s_yrsTillRepeatCode5) * s_rCode5TotTime;
        s_adjHoursCode6 = (1 / s_yrsTillRepeatCode6) * s_rCode6TotTime;
        s_adjHoursCode7 = (1 / s_yrsTillRepeatCode7) * s_rCode7TotTime;
        s_adjHoursCode8 = (1 / s_yrsTillRepeatCode8) * s_rCode8TotTime;
        s_totAdjHours = s_adjHoursCode3 + s_adjHoursCode4 + s_adjHoursCode5 + s_adjHoursCode6 +s_adjHoursCode7 + s_adjHoursCode8;
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
        s_totalAdjRatedQty = (s_yrsTillRepeatCode3factor * s_rCode3TotTrackQty)+(s_yrsTillRepeatCode4factor * s_rCode4TotTrackQty)
                + (s_yrsTillRepeatCode5factor * s_rCode5TotTrackQty) +(s_yrsTillRepeatCode6factor * s_rCode6TotTrackQty)
                +(s_yrsTillRepeatCode7factor * s_rCode7TotTrackQty) + (s_yrsTillRepeatCode8factor * s_rCode8TotTrackQty);
        //Print verbose results to console
        if (Constants::kVerbose) {
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
            std::cout << "Percentage of track time for scheduling rating code 3 - s_ratingRatio3 * 100 : "<< s_ratingRatio3 * Constants::kConvertDecimalToPercentDisplay << "%" << std::endl;
            std::cout << "Percentage of track time for scheduling rating code 4 - s_ratingRatio4 * 100 : "<< s_ratingRatio4  * Constants::kConvertDecimalToPercentDisplay << "%" << std::endl;
            std::cout << "Percentage of track time for scheduling rating code 5 - s_ratingRatio5 * 100 : "<< s_ratingRatio5  * Constants::kConvertDecimalToPercentDisplay << "%" << std::endl;
            std::cout << "Percentage of track time for scheduling rating code 6 - s_ratingRatio6 * 100 : "<< s_ratingRatio6 * Constants::kConvertDecimalToPercentDisplay <<  "%" << std::endl;
            std::cout << "Percentage of track time for scheduling rating code 7 - s_ratingRatio7 * 100 : "<< s_ratingRatio7 * Constants::kConvertDecimalToPercentDisplay <<  "%" << std::endl;
            std::cout << "Percentage of track time for scheduling rating code 8 - s_ratingRatio8 * 100 : "<< s_ratingRatio8 * Constants::kConvertDecimalToPercentDisplay <<  "%" << std::endl;
            std::cout << "Number of days until track repeat under rating code 3 - s_DaysBeforeRepeatCode3 : "<< s_DaysBeforeRepeatCode3 << std::endl;
            std::cout << "Average length of rated songs in fractional minutes - s_AvgMinsPerSong : "<< s_AvgMinsPerSong << std::endl;
            std::cout << "Calculated daily listening rate in mins - s_avgListeningRateInMins : "<< s_avgListeningRateInMins << std::endl;
            std::cout << "Calculated tracks per day - s_avgListeningRateInMins / s_AvgMinsPerSong : "<< s_avgListeningRateInMins / s_AvgMinsPerSong << std::endl;
            std::cout << "Sequential Track Limit - s_SequentialTrackLimit : "<< s_SequentialTrackLimit << std::endl<< std::endl;
        }
        s_bool_dbStatsCalculated = true; // Set bool to true for s_bool_dbStatsCalculated

        ui->daystracksLabel->setText(QString::number((m_prefs.tracksToAdd * s_AvgMinsPerSong)/s_avgListeningRateInMins,'g', 3));
    }
    else {
        s_bool_dbStatsCalculated = false;
        if (s_bool_IsUserConfigSet){
            std::cout << "Step 6. Something went wrong at function getDBStats." << std::endl;
        }
    }

    // Step 7a. If user configuration exists, MM.DB exists, songs table exists, statistics are processed, and
    //MM.DB was not recently updated, check for state of s_bool_artistsadjExist (artistsadj.txt).
    //If file is missing or empty, create file with artist statistics
    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist) &&
            (s_bool_dbStatsCalculated) && (!s_bool_MMdbUpdated)) {
        s_bool_artistsadjExist = false;
        bool tmpbool;
        bool tmpbool2;
        tmpbool = doesFileExist(appDataPathstr.toStdString()+"/artistsadj.txt");
        if (tmpbool){ // check that file is not empty
            //Check whether the songs table currently has any data in it
            std::streampos artsistAdjsize;
            char * memblock;
            std:: ifstream file (appDataPathstr.toStdString()+"/artistsadj.txt", std::ios::in|std::ios::binary|std::ios::ate);
            if (file.is_open())
            {
                artsistAdjsize = file.tellg();
                memblock = new char [static_cast<unsigned long>(artsistAdjsize)];
                file.seekg (0, std::ios::beg);
                file.read (memblock, artsistAdjsize);
                file.close();
                delete[] memblock;
            }
            if (artsistAdjsize != 0) {s_bool_artistsadjExist = true;// file artistsadj.txt exists and is greater in size than zero, set to true
                // If MM.DB not recently updated and artistsadj.txt does not need to be updated, check if ratedabbr.txt exists
                // If it does set s_bool_RatedAbbrExist to true.
                if (Constants::kVerbose) {std::cout << "Step 7. MM.DB not recently updated and artistsadj.txt does not need to be updated. "
                                                              "Now checking s_bool_RatedAbbrExist." << std::endl;}
                tmpbool2 = doesFileExist(appDataPathstr.toStdString()+"/ratedabbr.txt");
                if (tmpbool2){ // check that file is not empty
                    //Check whether the songs table currently has any data in it
                    std::streampos ratedabbrsize;
                    char * memblock;
                    std:: ifstream file (appDataPathstr.toStdString()+"/ratedabbr.txt", std::ios::in|std::ios::binary|std::ios::ate);
                    if (file.is_open())
                    {
                        ratedabbrsize = file.tellg();
                        memblock = new char [static_cast<unsigned long>(ratedabbrsize)];
                        file.seekg (0, std::ios::beg);
                        file.read (memblock, ratedabbrsize);
                        file.close();
                        delete[] memblock;
                    }
                    if (ratedabbrsize != 0) {
                        s_bool_RatedAbbrExist = true;
                        if (Constants::kVerbose) {std::cout << "Step 7. Set s_bool_RatedAbbrExist = true." << std::endl;}
                    }
                }
            }
            if (artsistAdjsize == 0) {s_bool_artistsadjExist = false;}// file exists but size is zero, set to false
        }
        if (!tmpbool){s_bool_artistsadjExist = false;} // file does not exist, set bool to false

        if (Constants::kVerbose) std::cout << "Step 7. MM.DB not recently updated. Verifying artistsadj.txt exists and is not zero. "
                                                     "s_bool_artistsadjExist result: "<< s_bool_artistsadjExist << std::endl;
        if (!s_bool_artistsadjExist){
            getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                                   &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                                   &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                                   &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);
            s_bool_artistsadjExist = doesFileExist (appDataPathstr.toStdString()+"/artistsadj.txt");
            s_bool_RatedAbbrExist = false;
            if (!s_bool_artistsadjExist)  {std::cout << "Step 7(a) Something went wrong at the function getArtistAdjustedCount. artistsadj.txt not created." << std::endl;}
        }
    }

    // Step 7b. If user configuration exists, MM.DB exists, songs table exists, statistics are processed, and
    // MM.DB was recently updated, create file with artist statistics

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist) &&
            (s_bool_dbStatsCalculated) && (s_bool_MMdbUpdated)) {
        if (Constants::kVerbose) {std::cout << "Step 7. MM.DB was recently updated. Processing artist statistics..." << std::endl;}
        getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                               &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                               &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                               &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);
        s_bool_RatedAbbrExist = false;
        s_bool_artistsadjExist = doesFileExist (appDataPathstr.toStdString()+"/artistsadj.txt");
        if (!s_bool_artistsadjExist)  {std::cout << "Step 7(b). Something went wrong at the function getArtistAdjustedCount. artistsadj.txt not created." << std::endl;}
    }

    // 8.  If user configuration exists, MM.DB exists, songs table exists, database statistics exist, artist statistics are processed, create
    // a modified database with only rated tracks and which include artist intervals calculated for each: If user configuration exists,
    // MM4 data exists, songs table exists, database statistics exist, and file artistsadj.txt is created (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist,
    // s_bool_dbStatsCalculated, bool10 are all true), run function getSubset() to create a modified database file with rated tracks
    // only and artist intervals for each track, rechecking, run doesFileExist (const std::string& name) (ratedabbr.txt) function (s_bool_RatedAbbrExist)

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist)  && (s_bool_dbStatsCalculated)
            && (s_bool_artistsadjExist) && (!s_bool_RatedAbbrExist)) {
        getSubset();
        s_bool_RatedAbbrExist = doesFileExist (appDataPathstr.toStdString()+"/ratedabbr.txt");
        if (!s_bool_RatedAbbrExist)  {std::cout << "Step 8. Something went wrong at the function getSubset(). ratedabbr.txt not created." << std::endl;}
        if ((s_bool_RatedAbbrExist) && (Constants::kVerbose)){std::cout << "Step 8. ratedabbr.txt was created." << std::endl;}
    }
    if ((Constants::kVerbose) && (s_bool_RatedAbbrExist) && (s_bool_IsUserConfigSet)){std::cout
                << "Step 8. MM.DB and artist.adj not recently updated. ratedabbr.txt not updated." << std::endl;}

    // 9. Determine if a playlist exists, and if not, determine if it was identified as being selected in the user's config:
    // Determine if cleaned (path-corrected) playlist selected (s_bool_PlaylistExist) cleanedplaylist.txt exists, doesFileExist (const std::string& name);
    // sets bool6 and bool7
    if (s_bool_IsUserConfigSet){
        s_bool_PlaylistExist = doesFileExist (appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        if (s_bool_PlaylistExist) {
            s_bool_PlaylistSelected = true;
            if (Constants::kVerbose){std::cout << "Step 9. Playlist exists and was not updated." << std::endl;}
        }
        //a. If s_bool_PlaylistExist is false, determine if playlist was identified as selected in user config (sets s_bool_PlaylistSelected)
        if (!s_bool_PlaylistExist){
            if (Constants::kVerbose){std::cout << "Step 9. Playlist not found. Checking user config for playlist selection." << std::endl;}
            std::string s_selectedplaylist = s_defaultPlaylist.toStdString();
            if (s_selectedplaylist != "") {
                s_bool_PlaylistSelected = true;
                if (Constants::kVerbose){std::cout << "Step 9. Playlist found in user config." << std::endl;}
            }
        }
    }
    // 10. If a playlist was identified in the user config, but the playlist file is not found, obtain the playlist file: If user configuration
    // exists, MM4 data exists, songs table exists (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist are all true), and playlist from user config exists
    // (s_bool_PlaylistSelected is true), but cleaned playlist does not (s_bool_PlaylistExist is false), run function to obtain cleaned playlist file getPlaylist()
    // then set s_bool_PlaylistExist to true, rechecking, run doesFileExist (const std::string& name) function. Evaluates s_bool_PlaylistExist and sets to true
    // (after running getPlaylist) if initially false
    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist) && (s_bool_PlaylistSelected) && (!s_bool_PlaylistExist)){
        if (Constants::kVerbose){std::cout << "Step 10. Playlist missing, but was found in user config. Recreating playlist" << std::endl;}
        getPlaylist(s_defaultPlaylist, s_musiclibrarydirname);
        s_bool_PlaylistExist = doesFileExist (appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        if (!s_bool_PlaylistExist) {std::cout << "Step 10. Something went wrong at the function getPlaylist." << std::endl;}
    }
    if ((Constants::kVerbose) && (s_bool_PlaylistExist) && (s_bool_IsUserConfigSet)){std::cout << "Step 10. Playlist exists and was not updated." << std::endl;}

    // NOTE: functions used in the next  three steps (11-13) will later be reused when adding tracks to
    // playlist - here is to get the initial values if a playlist exists

    //11. If playlist exists, calculate the playlist size: If cleaned playlist exists (s_bool_PlaylistExist is true), obtain playlist size
    // using function cstyleStringCount(),  s_playlistSize = cstyleStringCount(cleanedPlaylist);
    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet)) {
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        if (Constants::kVerbose){std::cout << "Step 11. Playlist size is: "<< s_playlistSize << std::endl;}
    }

    // 12. If playlist exists, obtain the historical count (in addition to the playlist count) up to the sequential track limit:
    // If cleaned playlist exists (s_bool_PlaylistExist is true), obtain the historical count (in addition to the playlist count) up to the
    // sequential track limit. A variable is needed (which later will be used to obtain additional play history outside of
    // playlist, as part of a later function to make a new track selection), using the variable s_histCount. The value is
    // calculated [ can be modified to use the function  to added function void getHistCount(&s_SequentialTrackLimit,&s_playlistSize),
    // or just: s_histCount = long(s_SequentialTrackLimit) – long(s_playlistSize); this uses both playlist size from 10
    // and SequentialTrackLimit obtained with data from function getDBStats()]
    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet)) {
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        s_histCount = int(s_SequentialTrackLimit - s_playlistSize);
        if (Constants::kVerbose){std::cout << "Step 12. s_histCount is: "<< s_histCount << std::endl;}
    }

    //13. If playlist exists, artist statistics are processed, and modified database exists, create/update excluded artists
    // list: If cleaned playlist exists (s_bool_PlaylistExist is true), and artistsadj.txt exists (s_bool_artistsadjExist is true) and modified database exists
    // (s_bool_RatedAbbrExist), run function getExcludedArtists() to create/update excluded artists list using vectors
    // read in from the following files: cleanlib.dsv, artistsadj.txt, and cleanedplaylist.txt. Writes artistexcludes.txt. Also,
    // creates temporary database (ratedabbr2.txt) with playlist position numbers for use in subsequent functions,
    //ratingCodeSelected and selectTrack
    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet))   {
        getExcludedArtists(s_playlistSize);
    }
    if ((s_includeNewTracks && s_bool_PlaylistExist)){  // If user is including new tracks, determine if a code 1 track should be added for this particular selection
        code1stats(&s_uniqueCode1ArtistCount,&s_code1PlaylistCount, &s_lowestCode1Pos, &s_artistLastCode1);// Retrieve rating code 1 stats
        ui->newtracksqtyLabel->setText(tr("New tracks qty not in playlist: ") + QString::number(s_rCode1TotTrackQty - s_code1PlaylistCount));
    }
    if (s_bool_IsUserConfigSet){
        ui->currentplsizeLabel->setText(tr("Current playlist size is ") + QString::number(s_playlistSize)+tr(" tracks, "));
        ui->playlistdaysLabel->setText(tr("and playlist length in listening days is ") +
                                       QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
        ui->repeatFreq1SpinBox->setValue(m_prefs.repeatFreqCode1);
        ui->addtrksspinBox->setValue(m_prefs.tracksToAdd);
        ui->newtracksqtyLabel->setText(tr("New tracks qty not in playlist: ") + QString::number(s_rCode1TotTrackQty - s_code1PlaylistCount));
        ui->factor3horizontalSlider->setMinimum(10);
        ui->factor3horizontalSlider->setMaximum(120);
        ui->factor3horizontalSlider->setValue(int(s_daysTillRepeatCode3));
        ui->factor3IntTxtLabel->setNum(s_daysTillRepeatCode3);
        ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
        ui->factor4doubleSpinBox->setValue(m_prefs.s_repeatFactorCode4);
        ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
        ui->factor5doubleSpinBox->setValue(m_prefs.s_repeatFactorCode5);
        ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
        ui->factor6doubleSpinBox->setValue(m_prefs.s_repeatFactorCode6);
        ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
        ui->factor7doubleSpinBox->setValue(m_prefs.s_repeatFactorCode7);
        ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
        ui->factor8doubleSpinBox->setValue(m_prefs.s_repeatFactorCode8);
        ui->playlistdaysLabel->setText(tr("and playlist length in listening days is ") +
                                       QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
        ui->daystoaddLabel->setText(tr("Based on a daily listening rate of ") + QString::number(s_avgListeningRateInMins,'g', 3)
                                    + tr(" minutes per day, tracks per day is ") + QString::number((s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3)+tr(", so"));
        ui->daystracksLabel->setText(tr("days added for 'Add Songs' quantity selected above will be ") +
                                     QString::number((m_prefs.tracksToAdd * s_AvgMinsPerSong)/s_avgListeningRateInMins,'g', 3)+tr(" days."));
        ui->label_perc5->setText(QString::number((((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime)/s_totAdjHours) * Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->label_perc4->setText(QString::number((((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime)/s_totAdjHours) * Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->label_perc35->setText(QString::number((((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime)/s_totAdjHours) * Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->label_perc3->setText(QString::number((((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime)/s_totAdjHours) * Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->label_perc25->setText(QString::number((((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime)/s_totAdjHours) * Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->label_perc2->setText(QString::number((((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)/s_totAdjHours) * Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->yearsradioButton->click();
        ui->playlistTab->setEnabled(true);
        ui->statisticsTab->setEnabled(true);
        ui->frequencyTab->setEnabled(true);
        if (m_prefs.s_includeAlbumVariety){
            ui->mainQTabWidget->setTabEnabled(4, true);
            ui->albumsTab->setEnabled(true);
        }
        if (!m_prefs.s_includeAlbumVariety){
            ui->mainQTabWidget->setTabEnabled(4, false);
            ui->albumsTab->setEnabled(false);
        }
        if (!m_prefs.s_includeNewTracks){
            ui->repeatFreq1SpinBox->setEnabled(false);
            ui->newtracksqtyLabel->setDisabled(true);
            ui->repeatfreqtxtLabel->setDisabled(true);
        }
        if (m_prefs.s_includeNewTracks){
            ui->repeatFreq1SpinBox->setEnabled(true);
            ui->newtracksqtyLabel->setDisabled(false);
            ui->repeatfreqtxtLabel->setDisabled(false);
        }
        if (m_prefs.s_noAutoSave){
            ui->autosavecheckBox->setChecked(true);
        }
        if (!m_prefs.s_noAutoSave){
            ui->autosavecheckBox->setChecked(false);
        }
        ui->mainQTabWidget->setTabEnabled(5, false);// unused tab
    }
        ui->minalbumsspinBox->setValue(m_prefs.s_minalbums);
        ui->mintracksspinBox->setValue(m_prefs.s_mintracks);
        ui->mintrackseachspinBox->setValue(m_prefs.s_mintrackseach);
        ui->totratedtimefreqLabel->setText("Total time (in hours) is: " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
        ui->totadjhoursfreqLabel->setText("Total adjusted time (in hours) is: " + QString::number(((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
                                                                                       ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
                                                                                       ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
                                                                                       ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
                                                                                       ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
                                                                                       ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)));
        ui->labelfreqperc5->setText(QString::number((((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc4->setText(QString::number((((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc35->setText(QString::number((((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc3->setText(QString::number((((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc25->setText(QString::number((((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc2->setText(QString::number((((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    if (!s_bool_IsUserConfigSet){
        ui->mainQTabWidget->setTabEnabled(0, false);
        ui->mainQTabWidget->setTabEnabled(2, false);
        ui->mainQTabWidget->setTabEnabled(3, false);
        ui->mainQTabWidget->setTabEnabled(4, false);
        ui->mainQTabWidget->setTabEnabled(5, false);
        ui->settingsTab->setEnabled(true);
    }
    /* 14. If user selects bool for s_includeAlbumVariety, run function buildAlbumExclLibrary(const int &s_minalbums,
      const int &s_mintrackseach, const int &s_mintracks). What it does: When basiclibrary functions are processed
      (at startup), run this function to get artists meeting the screening criteria (if user selected album variety).
      This generates the file artistalbmexcls.txt (see albumexcludes project)*/

    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet) && (s_includeAlbumVariety))
    {
        buildAlbumExclLibrary(s_minalbums, s_mintrackseach, s_mintracks);
        ui->albumsTab->setEnabled(true);
    }
}

void ArchSimian::on_addsongsButton_released(){
    if (Constants::kVerbose) std::cout << "Starting addSongs function." << std::endl;
    // First set messages and feedback to user during process

    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    int numTracks = ui->addtrksspinBox->value(); // Sets the number of tracks the user selected to add (numtracks)    
    if (numTracks > 29) {
        if (!s_disableNotificationAddTracks){
        QMessageBox msgBox;
        QString msgboxtxt = "This can take some time since you are adding " + QString::number(numTracks) + " tracks.";
        msgBox.setText(msgboxtxt);
        msgBox.exec();
        }
    }
    std::ofstream ofs; //open the songtext file for writing with the truncate option to delete the content.
    ofs.open(appDataPathstr.toStdString()+"/songtext.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    //removeAppData ("songtext.txt");
    std::ofstream songtext(appDataPathstr.toStdString()+"/songtext.txt",std::ios::app); // output file append mode for writing final song selections (ui display)
    // Second, determine the rating for the track selection
    if (Constants::kVerbose) std::cout << "Running ratingCodeSelected function before loop."<< std::endl;
    s_ratingNextTrack = ratingCodeSelected(s_ratingRatio3,s_ratingRatio4,s_ratingRatio5,s_ratingRatio6,
                                           s_ratingRatio7,s_ratingRatio8);
    if (Constants::kVerbose) std::cout <<"ratingCodeSelected function before loop completed. Result is: "<< s_ratingNextTrack <<
                                                ". Now starting loop for track selections..." <<std::endl;
    // Third, start loop for the number of tracks the user selected to add (numtracks)
    for (int i=0; i < numTracks; i++){
        if (Constants::kVerbose) std::cout << "Top of Loop. Count: " <<i<< std::endl;
        s_uniqueCode1ArtistCount = 0;
        s_code1PlaylistCount = 0;
        s_lowestCode1Pos = 99999;
        s_selectedCode1Path = "";
        s_selectedTrackPath = "";
        if (s_includeNewTracks){  // If user is including new tracks, determine if a code 1 track should be added for this particular selection
            code1stats(&s_uniqueCode1ArtistCount,&s_code1PlaylistCount, &s_lowestCode1Pos, &s_artistLastCode1);// Retrieve rating code 1 stats
            // Use stats to check that all code 1 tracks are not already in the playlist, and the repeat frequency is met
            if ((s_code1PlaylistCount < s_rCode1TotTrackQty) && ((s_lowestCode1Pos + 1) > s_repeatFreqForCode1)){
                getNewTrack(s_artistLastCode1, &s_selectedCode1Path); // Get rating code 1 track selection if criteria is met
                s_selectedTrackPath = s_selectedCode1Path; // set the track selection to the code 1 selection
                if (Constants::kVerbose) std::cout << "Rating code 1 applies to current track selection: " << s_selectedTrackPath << std::endl <<
                                                             "Code 1 track added to playlist."<< std::endl;
            }
        }
        else {s_selectedCode1Path = nullptr;} // If selection criteria for rating code 1 is not met, return empty string
        if (!s_selectedCode1Path.empty()) {
            s_ratingNextTrack = 1;} // If string is not empty, set rating for next track as code 1
        if ((!s_includeNewTracks)||(s_ratingNextTrack != 1)) { // If user excluded new tracks, or set rating code is not 1, do normal selection
            if ((Constants::kVerbose)&&(!s_includeNewTracks)) std::cout << "User excluding new tracks. Check whether user selected album variety " << std::endl;
            if (s_includeAlbumVariety){ // If not 1, and user has selected album variety, get album ID stats
                if (Constants::kVerbose) std::cout << "User selected album variety. Getting functions getTrimArtAlbmList and getAlbumIDs." << std::endl;
                getTrimArtAlbmList();
                getAlbumIDs();
            }
            if (Constants::kVerbose) std::cout << "Now selecting track for non-code-1 track selection (function selectTrack)." << std::endl;
            selectTrack(s_ratingNextTrack,&s_selectedTrackPath,s_includeAlbumVariety); // Select track if not a code 1 selection
        }
        // Collect track selected info for final song selections (ui display)
        std::string shortselectedTrackPath;
        shortselectedTrackPath = s_selectedTrackPath;
        std::string key1 ("/");
        std::string key2 ("_");
        shortselectedTrackPath.erase(0,11);
        std::size_t found = shortselectedTrackPath.rfind(key1);
        std::size_t found1 = shortselectedTrackPath.rfind(key2);
        if (found!=std::string::npos){shortselectedTrackPath.replace (found,key1.length(),", ");}
        if (found1!=std::string::npos){shortselectedTrackPath.replace (found,key2.length(),"");}
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        songtext << s_playlistSize<<". "<< shortselectedTrackPath <<'\n'; // adds the playlist pos number and track to the text display file
        if (Constants::kVerbose) std::cout << "Playlist length is: " << s_playlistSize << " tracks." << std::endl;
        // Calculate excluded artists and get rating for next track selection (accounting for track just added)
        s_histCount = long(s_SequentialTrackLimit) - long(s_playlistSize); // Recalc historical count (outside playlist count) up to sequential track limit
        getExcludedArtists(s_playlistSize); // Recalc excluded artists
        if (Constants::kVerbose) std::cout << "Running ratingCodeSelected function in loop."<< std::endl;
        s_ratingNextTrack = ratingCodeSelected(s_ratingRatio3,s_ratingRatio4,s_ratingRatio5,s_ratingRatio6,
                                               s_ratingRatio7,s_ratingRatio8); // Recalc rating selection
        if (Constants::kVerbose) std::cout<< "ratingCodeSelected function in loop completed. Result: " << s_ratingNextTrack << std::endl;
    }
     //After all tracks have been processed, update ui with information to user about tracks added to playlist
    if (s_includeNewTracks){  // If user is including new tracks, determine if a code 1 track should be added for this particular selection
        ui->newtracksqtyLabel->setText(tr("New tracks qty not in playlist: ") + QString::number(s_rCode1TotTrackQty - s_code1PlaylistCount));
    }
    songtext.close();
    ui->currentplsizeLabel->setText(tr("Current playlist size is ") + QString::number(s_playlistSize)+tr(" tracks, "));
    ui->playlistdaysLabel->setText(tr("and playlist length in listening days is ") +
                                          QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
    ui->statusBar->showMessage("Added " + QString::number(numTracks) + " tracks to playlist",100000);
    QFile songtext1(appDataPathstr+"/songtext.txt");
    if(!songtext1.open(QIODevice::ReadOnly))
        QMessageBox::information(nullptr,"info",songtext1.errorString());
    QTextStream in(&songtext1);
    ui->songsaddtextBrowser->setText(in.readAll());
    ui->addsongsButton->setEnabled(true);
}

void ArchSimian::on_exportplaylistButton_clicked(){
    int s_musicdirlength{};
    s_musicdirlength = musicLibraryDirLen(s_musiclibrarydirname);
    exportPlaylistToWindows(s_musicdirlength, s_mmPlaylistDir,  s_defaultPlaylist,  s_winDriveLtr,  s_musiclibrarydirname);
    ui->statusBar->showMessage("Replaced Windows playlist with Archsimian-modified playlist",50000);
}

void ArchSimian::on_setlibraryButton_clicked(){
    QFileDialog setlibraryButton;
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
    ui->setmmplButton->setEnabled(true);
    //      }
}

void ArchSimian::on_setmmplButton_clicked(){
    QFileDialog setmmpldialog;
    setmmpldialog.setFileMode(QFileDialog::Directory);
    setmmpldialog.setOption(QFileDialog::ShowDirsOnly);
    const QString mmbackuppldirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select MediaMonkey Playlist Backup Directory"),
                "/"
                );
    ui->setmmpllabel->setText(QString(mmbackuppldirname));
    m_prefs.mmPlaylistDir = mmbackuppldirname;
}

void ArchSimian::on_setmmdbButton_clicked(){
    QFileDialog setmmdbdialog;
    setmmdbdialog.setFileMode(QFileDialog::Directory);
    setmmdbdialog.setOption(QFileDialog::ShowDirsOnly);
    const QString mmbackupdbdirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select MediaMonkey Database (MM.DB) Backup Directory"),
                "/"
                );
    ui->setmmdblabel->setText(QString(mmbackupdbdirname));
    m_prefs.mmBackupDBDir = mmbackupdbdirname;
}

// User selects playlist from configured directory for 'backup playlists'
void ArchSimian::on_getplaylistButton_clicked()
{
    QFileDialog setgetplaylistdialog;
    QString selectedplaylist = QFileDialog::getOpenFileName (
                this,
                "Select playlist for which you will add tracks",
                QString(s_mmPlaylistDir),//default dir for playlists
                "playlists(.m3u) (*.m3u)");

        if (selectedplaylist.size() > 0)
        {
        m_prefs.defaultPlaylist = selectedplaylist;
        s_defaultPlaylist = m_prefs.defaultPlaylist;
        ui->setgetplaylistLabel->setText("Selected: " + QString(selectedplaylist));
        getPlaylist(s_defaultPlaylist, s_musiclibrarydirname);
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        s_histCount = int(s_SequentialTrackLimit - s_playlistSize);
        getExcludedArtists(s_playlistSize);
        if (s_includeAlbumVariety){
            buildAlbumExclLibrary(s_minalbums, s_mintrackseach, s_mintracks);
        }
        ui->currentplsizeLabel->setText(tr("Current playlist size is ") + QString::number(s_playlistSize)+tr(" tracks, "));
        ui->playlistdaysLabel->setText(tr("and playlist length in listening days is ") + QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
        if (s_includeNewTracks){  // If user is including new tracks, determine if a code 1 track should be added for this particular selection
            s_uniqueCode1ArtistCount = 0;
            s_code1PlaylistCount = 0;
            s_lowestCode1Pos = 99999;
            code1stats(&s_uniqueCode1ArtistCount,&s_code1PlaylistCount, &s_lowestCode1Pos, &s_artistLastCode1);// Retrieve rating code 1 stats
            ui->newtracksqtyLabel->setText(tr("New tracks qty not in playlist: ") + QString::number(s_rCode1TotTrackQty - s_code1PlaylistCount));
        }
        ui->songsaddtextBrowser->setText("");
        ui->addsongsButton->setEnabled(true);
        ui->exportplaylistButton->setEnabled(true);
        }

}

void ArchSimian::on_mainQTabWidget_tabBarClicked(int index)
{
    if (index == 0) { // if the Playlist tab is selected, save and reload settings since rating code values may have been changed
        saveSettings();
        loadSettings();
        }
    if (index == 2) // if the Statistics tab is selected, refresh stats
    {
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
        ui->cd1trcktimeLabel->setText("Code 1 - Total tracks: " + QString::number(s_rCode1TotTrackQty)+ ", Tot hours: "
                                      + QString::number((s_rCode1MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor));
        ui->cd3trcktimeLabel->setText("Code 3 - Tot tracks: " + QString::number(s_rCode3TotTrackQty) + ", Tot hours: "
                                      + QString::number((s_rCode3MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor)+", Adj (for repeat freq) tot hours: "
                                      + QString::number((1 / s_yrsTillRepeatCode3) * (s_rCode3MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor));
        ui->cd4trcktimeLabel->setText("Code 4 - Tot tracks: " + QString::number(s_rCode4TotTrackQty) + ", Tot hours: "
                                      + QString::number((s_rCode4MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor)+", Adj (for repeat freq) tot hours: "
                                      + QString::number((1 / s_yrsTillRepeatCode4) * (s_rCode4MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor));
        ui->cd5trcktimeLabel->setText("Code 5 - Tot tracks: " + QString::number(s_rCode5TotTrackQty) + ", Tot hours: "
                                      + QString::number((s_rCode5MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor)+", Adj (for repeat freq) tot hours: "
                                      + QString::number((1 / s_yrsTillRepeatCode5) * (s_rCode5MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor));
        ui->cd6trcktimeLabel->setText("Code 6 - Tot tracks: " + QString::number(s_rCode6TotTrackQty) + ", Tot hours: "
                                      + QString::number((s_rCode6MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor)+", Adj (for repeat freq) tot hours: "
                                      + QString::number((1 / s_yrsTillRepeatCode6) * (s_rCode6MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor));
        ui->cd7trcktimeLabel->setText("Code 7 - Tot tracks: " + QString::number(s_rCode7TotTrackQty) + ", Tot hours: "
                                      + QString::number((s_rCode7MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor)+", Adj (for repeat freq) tot hours: "
                                      + QString::number((1 / s_yrsTillRepeatCode7) * (s_rCode7MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor));
        ui->cd8trcktimeLabel->setText("Code 8 - Tot tracks: " + QString::number(s_rCode8TotTrackQty) + ", Tot hours: "
                                      + QString::number((s_rCode8MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor)+", Adj (for repeat freq) tot hours: "
                                      + QString::number((1 / s_yrsTillRepeatCode8) * (s_rCode8MsTotTime/Constants::kMilSecsToMinsFactor)/Constants::kMinsToHoursFactor));
        ui->totadjhoursLabel->setText("Tot adjusted hours: " + QString::number(((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
                                                                               ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
                                                                               ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
                                                                               ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
                                                                               ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
                                                                               ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)));
        ui->totadjtracksLabel->setText("Tot adjusted tracks: " + QString::number(s_totalAdjRatedQty));
        ui->labelfreqperc5->setText(QString::number((((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc4->setText(QString::number((((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc35->setText(QString::number((((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc3->setText(QString::number((((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc25->setText(QString::number((((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
        ui->labelfreqperc2->setText(QString::number((((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");

    }
}
void ArchSimian::on_addtrksspinBox_valueChanged(int s_numTracks)
{
    m_prefs.tracksToAdd = s_numTracks;
    //s_listeningRate //double(s_AvgMinsPerSong*value)/s_avgListeningRateInMins)
    ui->daystoaddLabel->setText(tr("Based on a daily listening rate of ") + QString::number(s_avgListeningRateInMins,'g', 3)
                                + tr(" minutes per day, tracks per day is ") + QString::number((s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3)+tr(", so"));
    ui->daystracksLabel->setText(tr("days added for 'Add Songs' quantity selected above will be ") +
                                 QString::number((m_prefs.tracksToAdd * s_AvgMinsPerSong)/s_avgListeningRateInMins,'g', 3)+tr(" days."));
    if (s_numTracks > 29){
        s_disableNotificationAddTracks = false;
        m_prefs.s_disableNotificationAddTracks = s_disableNotificationAddTracks;
    }
}

void ArchSimian::on_repeatFreq1SpinBox_valueChanged(int myvalue)
{
    m_prefs.repeatFreqCode1 = myvalue;
    //s_repeatFreqForCode1 = ui->repeatFreq1SpinBox->value();
}

void ArchSimian::loadSettings()
{
    QSettings settings;
    m_prefs.repeatFreqCode1 = settings.value("repeatFreqCode1", Constants::kUserDefaultRepeatFreqCode1).toInt();
    m_prefs.tracksToAdd = settings.value("tracksToAdd", Constants::kUserDefaultTracksToAdd).toInt();
    m_prefs.defaultPlaylist = settings.value("defaultPlaylist", Constants::kUserDefaultDefaultPlaylist).toString();
    m_prefs.musicLibraryDir = settings.value("musicLibraryDir", "").toString();
    m_prefs.mmBackupDBDir = settings.value("mmBackupDBDir", "").toString();
    m_prefs.mmPlaylistDir = settings.value("mmPlaylistDir", "").toString();
    m_prefs.s_includeNewTracks = settings.value("includeNewTracks", Constants::kUserDefaultIncludeNewTracks).toBool();
    m_prefs.s_includeAlbumVariety = settings.value("s_includeAlbumVariety", Constants::kUserDefaultIncludeAlbumVariety).toBool();
    m_prefs.s_noAutoSave = settings.value("s_noAutoSave", Constants::kUserDefaultNoAutoSave).toBool();
    m_prefs.s_disableNotificationAddTracks = settings.value("s_disableNotificationAddTracks", Constants::kUserDefaultDisableNotificationAddTracks).toBool();
    m_prefs.s_daysTillRepeatCode3 = settings.value("s_daysTillRepeatCode3", Constants::kUserDefaultDaysTillRepeatCode3).toDouble();
    m_prefs.s_repeatFactorCode4 = settings.value("s_repeatFactorCode4", Constants::kUserDefaultRepeatFactorCode4).toDouble();
    m_prefs.s_repeatFactorCode5 = settings.value("s_repeatFactorCode5", Constants::kUserDefaultRepeatFactorCode5).toDouble();
    m_prefs.s_repeatFactorCode6 = settings.value("s_repeatFactorCode6", Constants::kUserDefaultRepeatFactorCode6).toDouble();
    m_prefs.s_repeatFactorCode7 = settings.value("s_repeatFactorCode7", Constants::kUserDefaultRepeatFactorCode7).toDouble();
    m_prefs.s_repeatFactorCode8 = settings.value("s_repeatFactorCode8", Constants::kUserDefaultRepeatFactorCode8).toDouble();
    m_prefs.s_WindowsDriveLetter = settings.value("s_WindowsDriveLetter",Constants::kUserDefaultWindowsDriveLetter).toString();
    m_prefs.s_minalbums = settings.value("s_minalbums", Constants::kUserDefaultMinalbums).toInt();
    m_prefs.s_mintrackseach = settings.value("s_mintrackseach", Constants::kUserDefaultMintrackseach).toInt();
    m_prefs.s_mintracks = settings.value("s_mintracks", Constants::kUserDefaultMintracks).toInt();
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
    settings.setValue("includeNewTracks",m_prefs.s_includeNewTracks);
    settings.setValue("s_noAutoSave",m_prefs.s_noAutoSave);    
    settings.setValue("s_disableNotificationAddTracks",m_prefs.s_disableNotificationAddTracks);
    settings.setValue("s_includeAlbumVariety",m_prefs.s_includeAlbumVariety);
    settings.setValue("s_daysTillRepeatCode3",m_prefs.s_daysTillRepeatCode3);
    settings.setValue("s_repeatFactorCode4",m_prefs.s_repeatFactorCode4);
    settings.setValue("s_repeatFactorCode5",m_prefs.s_repeatFactorCode5);
    settings.setValue("s_repeatFactorCode6",m_prefs.s_repeatFactorCode6);
    settings.setValue("s_repeatFactorCode7",m_prefs.s_repeatFactorCode7);
    settings.setValue("s_repeatFactorCode8",m_prefs.s_repeatFactorCode8);
    settings.setValue("s_minalbums",m_prefs.s_minalbums);
    settings.setValue("s_mintrackseach",m_prefs.s_mintrackseach);
    settings.setValue("s_mintracks",m_prefs.s_mintracks);
    settings.setValue("s_WindowsDriveLetter",m_prefs.s_WindowsDriveLetter);

}
void ArchSimian::closeEvent(QCloseEvent *event)
{
    event->ignore();
    if (s_noAutoSave == 0){
        saveSettings();
        event->accept();
    }
    if (s_noAutoSave == 1){
    if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Do you wish to save any changes made to settings before exit?",
                                                  QMessageBox::Yes | QMessageBox::No))
    {
        saveSettings();
        event->accept();
    }
    event->accept();
    }
}

void ArchSimian::on_daysradioButton_clicked()
{
    s_dateTranslation = Constants::kDaysInYear;
    dateTransTextVal = " days";
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * Constants::kDaysInYear,'g', 3) + dateTransTextVal);
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * Constants::kDaysInYear,'g', 3) + dateTransTextVal);
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * Constants::kDaysInYear,'g', 3) + dateTransTextVal);
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * Constants::kDaysInYear,'g', 3) + dateTransTextVal);
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * Constants::kDaysInYear,'g', 3) + dateTransTextVal);
}

void ArchSimian::on_weeksradioButton_clicked()
{
    s_dateTranslation = Constants::kWeeksInYear;
    dateTransTextVal = " weeks";
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * Constants::kWeeksInYear,'g', 3) + dateTransTextVal);
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * Constants::kWeeksInYear,'g', 3) + dateTransTextVal);
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * Constants::kWeeksInYear,'g', 3) + dateTransTextVal);
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * Constants::kWeeksInYear,'g', 3) + dateTransTextVal);
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * Constants::kWeeksInYear,'g', 3) + dateTransTextVal);
}

void ArchSimian::on_monthsradioButton_clicked()
{
    s_dateTranslation = Constants::kMonthsInYear;
    dateTransTextVal = " months";
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * Constants::kMonthsInYear,'g', 3) + dateTransTextVal);
}

void ArchSimian::on_yearsradioButton_clicked()
{
    s_dateTranslation = 1;
    dateTransTextVal = " years";
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3,'g', 3) + dateTransTextVal);
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4,'g', 3) + dateTransTextVal);
    ui->factor6label->setText(QString::number(m_prefs.s_repeatFactorCode6 * s_yrsTillRepeatCode5,'g', 3) + dateTransTextVal);
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6,'g', 3) + dateTransTextVal);
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7,'g', 3) + dateTransTextVal);
}

void ArchSimian::on_factor3horizontalSlider_valueChanged(int value)
{
    m_prefs.s_daysTillRepeatCode3 = value;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
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
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    ui->totratedtimefreqLabel->setText("Total time (in hours) is: " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
    ui->totadjhoursfreqLabel->setText("Total adjusted time (in hours) is: " + QString::number(((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)));
    ui->labelfreqperc5->setText(QString::number((((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc4->setText(QString::number((((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc35->setText(QString::number((((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc3->setText(QString::number((((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc25->setText(QString::number((((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc2->setText(QString::number((((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
}

void ArchSimian::on_factor4doubleSpinBox_valueChanged(double argfact4)
{
    m_prefs.s_repeatFactorCode4 = argfact4;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
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
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    ui->totratedtimefreqLabel->setText("Total time (in hours) is: " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
    ui->totadjhoursfreqLabel->setText("Total adjusted time (in hours) is: " + QString::number(((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)));
    ui->labelfreqperc5->setText(QString::number((((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc4->setText(QString::number((((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc35->setText(QString::number((((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc3->setText(QString::number((((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc25->setText(QString::number((((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc2->setText(QString::number((((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
}

void ArchSimian::on_factor5doubleSpinBox_valueChanged(double argfact5)
{
    m_prefs.s_repeatFactorCode5 = argfact5;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
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
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    ui->totratedtimefreqLabel->setText("Total time (in hours) is: " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
    ui->totadjhoursfreqLabel->setText("Total adjusted time (in hours) is: " + QString::number(((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)));
    ui->labelfreqperc5->setText(QString::number((((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc4->setText(QString::number((((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc35->setText(QString::number((((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc3->setText(QString::number((((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc25->setText(QString::number((((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc2->setText(QString::number((((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)/s_totAdjHours)*Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
}

void ArchSimian::on_factor6doubleSpinBox_valueChanged(double argfact6)
{
    m_prefs.s_repeatFactorCode6 = argfact6;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
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
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    ui->totratedtimefreqLabel->setText("Total time (in hours) is: " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
    ui->totadjhoursfreqLabel->setText("Total adjusted time (in hours) is: " + QString::number(((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)));
    ui->labelfreqperc5->setText(QString::number((((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc4->setText(QString::number((((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc35->setText(QString::number((((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc3->setText(QString::number((((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc25->setText(QString::number((((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc2->setText(QString::number((((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
}

void ArchSimian::on_factor7doubleSpinBox_valueChanged(double argfact7)
{
    m_prefs.s_repeatFactorCode7 = argfact7;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
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
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    ui->totratedtimefreqLabel->setText("Total time (in hours) is: " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
    ui->totadjhoursfreqLabel->setText("Total adjusted time (in hours) is: " + QString::number(((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)));
    ui->labelfreqperc5->setText(QString::number((((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc4->setText(QString::number((((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc35->setText(QString::number((((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc3->setText(QString::number((((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc25->setText(QString::number((((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc2->setText(QString::number((((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
}

void ArchSimian::on_factor8doubleSpinBox_valueChanged(double argfact8)
{
    m_prefs.s_repeatFactorCode8 = argfact8;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
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
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    ui->totratedtimefreqLabel->setText("Total time (in hours) is: " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
    ui->totadjhoursfreqLabel->setText("Total adjusted time (in hours) is: " + QString::number(((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
                                                                                   ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)));
    ui->labelfreqperc5->setText(QString::number((((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc4->setText(QString::number((((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc35->setText(QString::number((((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc3->setText(QString::number((((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc25->setText(QString::number((((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
    ui->labelfreqperc2->setText(QString::number((((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)/s_totAdjHours)* Constants::kConvertDecimalToPercentDisplay,'g', 3) + "%");
}

void ArchSimian::on_InclNewcheckbox_stateChanged(int inclNew)
{
    m_prefs.s_includeNewTracks = inclNew;
    ui->InclNewcheckbox->checkState();
    if (ui->InclNewcheckbox->checkState() == 2){
    ui->repeatFreq1SpinBox->setEnabled(true);
    ui->newtracksqtyLabel->setDisabled(false);
    ui->repeatfreqtxtLabel->setDisabled(false);
    QWidget::repaint();
    }
    if (ui->InclNewcheckbox->checkState() == false){
    ui->repeatFreq1SpinBox->setEnabled(false);
    ui->newtracksqtyLabel->setDisabled(true);
    ui->repeatfreqtxtLabel->setDisabled(true);
    QWidget::repaint();
    }
}

void ArchSimian::on_albumscheckBox_stateChanged(int inclAlbums)
{
    ui->albumscheckBox->checkState();
    m_prefs.s_includeAlbumVariety = inclAlbums;
    if (ui->albumscheckBox->checkState() == 2){
        ui->mainQTabWidget->setTabEnabled(4, true);
        ui->albumsTab->setEnabled(true);
        QWidget::repaint();
    }
    if (ui->albumscheckBox->checkState() == 0){
        ui->mainQTabWidget->setTabEnabled(4, false);
        ui->albumsTab->setEnabled(false);
        QWidget::repaint();
    }
}

void ArchSimian::on_minalbumsspinBox_valueChanged(int arg1)
{
   m_prefs.s_minalbums =  arg1;
   ui->mintracksspinBox->setMinimum(s_minalbums * s_mintrackseach);
}

void ArchSimian::on_mintracksspinBox_valueChanged(int arg1)
{
    m_prefs.s_mintracks = arg1;

}

void ArchSimian::on_mintrackseachspinBox_valueChanged(int arg1)
{
 m_prefs.s_mintrackseach = arg1;
 ui->mintracksspinBox->setMinimum(s_minalbums * s_mintrackseach);
}

void ArchSimian::on_addsongsButton_clicked(bool checked) // change button state when tracks added is completed
{
    ui->addsongsButton->setEnabled(false);
    ui->addtrksspinBox->setEnabled(false);
    if (checked == true){
        ui->addsongsButton->isDown();
        checked = false;
    }
    ui->addsongsButton->setEnabled(true);
    ui->addtrksspinBox->setEnabled(true);
}



void ArchSimian::on_actionExport_Playlist_triggered()
{
    int s_musicdirlength{};
    s_musicdirlength = musicLibraryDirLen(s_musiclibrarydirname);
    exportPlaylistToWindows(s_musicdirlength, s_mmPlaylistDir,  s_defaultPlaylist,  s_winDriveLtr,  s_musiclibrarydirname);
    ui->statusBar->showMessage("Replaced Windows playlist with Archsimian-modified playlist",50000);
}


void ArchSimian::on_actionExit_triggered()
{
    if (!s_noAutoSave){
        saveSettings();
       qApp->quit();
    }
    if (s_noAutoSave){
        if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Do you wish to save any changes to "
                                                      "the configuration settings before quitting?", QMessageBox::Yes | QMessageBox::No))
        {
            saveSettings();
        }
        qApp->quit();
    }
}

void ArchSimian::on_actionAbout_Qt_triggered()
{
 QApplication::aboutQt();
}

void ArchSimian::on_actionSave_Settings_triggered()
{
    saveSettings();
    ui->statusBar->showMessage("Saved user settings",50000);
}

void ArchSimian::on_actionAbout_triggered()
{
    QMessageBox::about(this,tr("ArchSimian") ,tr("\nArchSimian v.1.0-beta.18"
               "\n\nThis program is free software: you can redistribute it and/or modify"
               " it under the terms of the GNU General Public License as published by"
               " the Free Software Foundation, either version 3 of the License, or"
               " (at your option) any later version.\n"
               "\n"
               "This program is distributed in the hope that it will be useful,"
               " but WITHOUT ANY WARRANTY; without even the implied warranty of"
               " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
               " GNU General Public License for more details.\n"
               "\n"
               "You should have received a copy of the GNU General Public License"
               " along with this program. If not, see https://www.gnu.org/licenses/"));
}

void ArchSimian::on_actionOpen_Playlist_triggered()
{
ArchSimian::on_getplaylistButton_clicked();
}

void ArchSimian::on_autosavecheckBox_stateChanged(int autosave)
{
    s_noAutoSave = autosave;
    m_prefs.s_noAutoSave = s_noAutoSave;
}

void ArchSimian::on_disablenotecheckBox_stateChanged(int disableNote)
{
    s_disableNotificationAddTracks = disableNote;
    m_prefs.s_disableNotificationAddTracks = disableNote;
    QWidget::repaint();
}

void ArchSimian::on_resetpushButton_released()
{
    if (QMessageBox::Yes == QMessageBox::question(this, "Reset Confirmation", "Are you sure you want to reset "
                                                  "all of your preferences and exit the program?", QMessageBox::Yes | QMessageBox::No))
    {
        m_prefs.repeatFreqCode1 = Constants::kUserDefaultRepeatFreqCode1;
        m_prefs.tracksToAdd = Constants::kUserDefaultTracksToAdd;
        m_prefs.defaultPlaylist = Constants::kUserDefaultDefaultPlaylist;
        m_prefs.s_includeNewTracks = Constants::kUserDefaultIncludeNewTracks;
        m_prefs.s_includeAlbumVariety =  Constants::kUserDefaultIncludeAlbumVariety;
        m_prefs.s_noAutoSave = Constants::kUserDefaultNoAutoSave;
        m_prefs.s_disableNotificationAddTracks = Constants::kUserDefaultDisableNotificationAddTracks;
        m_prefs.s_daysTillRepeatCode3 = Constants::kUserDefaultDaysTillRepeatCode3;
        m_prefs.s_repeatFactorCode4 = Constants::kUserDefaultRepeatFactorCode4;
        m_prefs.s_repeatFactorCode5 = Constants::kUserDefaultRepeatFactorCode5;
        m_prefs.s_repeatFactorCode6 = Constants::kUserDefaultRepeatFactorCode6;
        m_prefs.s_repeatFactorCode7 = Constants::kUserDefaultRepeatFactorCode7;
        m_prefs.s_repeatFactorCode8 = Constants::kUserDefaultRepeatFactorCode8;
        m_prefs.s_WindowsDriveLetter = Constants::kUserDefaultWindowsDriveLetter;
        m_prefs.s_minalbums = Constants::kUserDefaultMinalbums;
        m_prefs.s_mintrackseach = Constants::kUserDefaultMintrackseach;
        m_prefs.s_mintracks = Constants::kUserDefaultMintracks;
        saveSettings();
        qApp->quit();
    }
}
