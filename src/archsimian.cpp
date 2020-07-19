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
#include "utilities.h"
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
#include "playlistcontentdialog.h"
#include "playlistlimit.h"
#include "diagnostics.h"

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
static int s_lowestCode1Pos{Constants::kMaxLowestCode1Pos};
static std::string s_artistLastCode1;
static std::string s_selectedCode1Path;
static std::string s_selectedTrackPath;
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
static int s_rCode0TotTime{0},s_rCode1TotTime{0},s_rCode3TotTime{0},s_rCode4TotTime{0},
s_rCode5TotTime{0},s_rCode6TotTime{0},s_rCode7TotTime{0},s_rCode8TotTime{0};
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
static double s_adjHoursCode3{0.0}, s_adjHoursCode4{0.0},s_adjHoursCode5{0.0},s_adjHoursCode6{0.0},
s_adjHoursCode7{0.0},s_adjHoursCode8{0.0};
static double s_DaysBeforeRepeatCode3{0.0};
static double s_totHrsLast60Days{0.0};
static double s_totalAdjRatedQty{0.0};
static int s_playlistSize{0};
static int s_ratingNextTrack{0};
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
static QString s_musiclibshortened{""};
static QString s_windowstopfolder{""};
static QString appDataPathstr = QDir::homePath() + "/.local/share/archsimian";
static QDir appDataPath = appDataPathstr;
static std::string cleanLibFile = appDataPathstr.toStdString()+"/cleanlib.dsv";
static int selectedTrackLimitCode{3};
static int selTrackLimitCodeTotTrackQty{0};
static double selTrackLimitCodeRatingRatio{0};
static double trackLimitPercentage{0.95};
static int s_playlistActualCntSelCode{0};
static int playlistTrackLimitCodeQty{0};
static bool playlistFull{false};
static int s_MaxAvailableToAdd{0};
static bool s_topLevelFolderExists{0};
static double s_MaxAvailStaticCast{0.0};
static std::string playlistpath{""};

// Create UI Widget ArchSimian - UI Set up
ArchSimian::ArchSimian(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ArchSimian)
{   
    QWidget setupwindow;
    m_sSettingsFile = QApplication::applicationDirPath().left(1) + ":/archsimian.conf"; // sets the config file location for QSettings
    // QSettings: load user settings from archsimian.conf file
    loadSettings();
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
    s_winDriveLtr = m_prefs.s_WindowsDriveLetter;
    s_windowstopfolder = m_prefs.s_windowstopfolder;
    s_musiclibshortened = m_prefs.s_musiclibshortened;
    //s_playlistActualCntSelCode = m_prefs.s_playlistActualCntSelCode;

    // Set up the UI
    ui->setupUi(this);
    ui->mainQTabWidget->setCurrentIndex(0);

    // Step 1. Determine if user configuration exists:
    //
    // Use QSettings archsimian.conf file entries to check entries for mmBackupDBDir, mmPlaylistDir, and musicLibraryDir
    // If any are blank, or the dir found does not exist, or file MM.DB does not exist at the location specified, then
    // launch child window for user to set the locations prior to launch of main window. When child window
    // is closed verify the locations and files selected exist.
    //
    // UI configuration: determine state of user config
    //
    if ((s_mmBackupDBDir != nullptr) && (s_musiclibrarydirname != nullptr) && (s_mmPlaylistDir != nullptr) &&(s_winDriveLtr != nullptr)){
        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 1. The locations s_mmBackupDBDir, s_musiclibrarydirname & mmPlaylistDir have all been set up." << std::endl;}
        s_bool_IsUserConfigSet = true;
    }
    else {        
        std::cout << "Archsimian.cpp: Step 1. Unable to open archsimian.conf configuration file or, one or more locations has no data. s_bool_IsUserConfigSet result: "
                  << s_bool_IsUserConfigSet << std::endl;
    }
    //If configuration has already been set but a library has not yet been processed (new config or MM.DB update), run diagnostics on the configuration completed
    if ((s_bool_IsUserConfigSet)&&(!s_bool_CleanLibExist)) {
    generateDiagsLog();
    if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 1. Generating a diagnostic check on configuration." << std::endl;
    }

    //If configuration has already been set, populate the UI labels accordingly
    if (s_bool_IsUserConfigSet)
    {
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 1. Configuration was set. s_bool_IsUserConfigSet result: " << s_bool_IsUserConfigSet << std::endl;
        m_prefs.musicLibraryDir = s_musiclibrarydirname;
        if (s_includeNewTracks) {ui->InclNewcheckbox->setChecked(true);}
        if (s_includeAlbumVariety) {
            ui->albumscheckBox->setChecked(true);
            ui->mainQTabWidget->setTabEnabled(4, true);
        }
        ui->saveConfigButton->setEnabled(false);
        ui->menuBar->setDisabled(false);
        ui->setlibrarylabel->setText(QString(s_musiclibrarydirname));
        ui->windowsDriveLtrEdit->setText(QString(s_winDriveLtr));
        ui->setlibraryButton->setEnabled(true);
        ui->setmmpllabel->setText(QString(s_defaultPlaylist));
        ui->setmmplButton->setEnabled(true);
        ui->setmmdblabel->setText(s_mmBackupDBDir);
        ui->setmmpllabel->setText(s_mmPlaylistDir);
        ui->setgetplaylistLabel->setText("Selected: " + s_defaultPlaylist);
        ui->viewplaylistButton->setDisabled(false);
        ui->viewplaylistLabel->setText(tr("View currently selected playlist"));
        ui->instructionlabel->setText(tr(""));
        ui->addsongsLabel->setText(tr("tracks to selected playlist."));
        if (s_defaultPlaylist == ""){
            ui->setgetplaylistLabel->setText("No playlist selected");
            ui->addsongsButton->setEnabled(false);
            ui->viewplaylistButton->setDisabled(true);
            ui->viewplaylistLabel->setText(tr("No playlist selected"));
            QMainWindow::setWindowTitle("ArchSimian - No playlist selected");
        }
        ui->setmmdbButton->setEnabled(true);
        bool needUpdate = recentlyUpdated(s_mmBackupDBDir);
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 1. Checking getMMdbDate(): "<< needUpdate << std::endl;
        if (needUpdate == 0)
        {
            std::string MMdbDate = getMMdbDate(s_mmBackupDBDir);
            std::string LastTableDate = getLastTableDate();
            ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(MMdbDate)+
                                           tr(", Library date: ")+ QString::fromStdString(LastTableDate));
        }
    }
    // Step 2. Determine if the MediaMonkey (MM.DB) database exists with the doesFileExist (const std::string& name) function (sets s_bool_MMdbExist).
    if (s_bool_IsUserConfigSet) {
        std::string mmdbdir = s_mmBackupDBDir.toStdString();
        const std::string mmpath = mmdbdir + "/MM.DB";
        s_bool_MMdbExist = doesFileExist(mmpath);
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 2. Does MM.DB file exist. s_bool_MMdbExist result: " << s_bool_MMdbExist << std::endl;
    }
    //    a. If s_bool_IsUserConfigSet is false, set all three location variables to false and s_winDriveLtr to ""
    if (!s_bool_IsUserConfigSet) {
        s_bool_MMdbExist = false;
        s_bool_MMdbUpdated = false;
        s_bool_PlaylistExist = false;
        s_winDriveLtr = "";
    }
    //    b. If s_bool_IsUserConfigSet is true, but MM.DB was not found, set s_bool_IsUserConfigSet to false
    if ((s_bool_IsUserConfigSet) && (!s_bool_MMdbExist)) {
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 2. MM.DB was not found at the location you specified. Setting s_bool_IsUserConfigSet to false." << std::endl;
        s_bool_IsUserConfigSet = false;
    }
    // If configuration has not been set up completely, load UI setup instructions for user to set configuration locations
    if (!s_bool_IsUserConfigSet) {
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 1. Configuration has not been set. Adjusting UI settings" << std::endl;
        QMainWindow::setWindowTitle("ArchSimian - Initial Configuration Setup");
        ui->setlibrarylabel->setText(tr(""));
        ui->setlibraryButton->setEnabled(true);
        ui->setmmpllabel->setText(tr(""));
        ui->setmmplButton->setEnabled(true);
        ui->setmmdblabel->setText(tr(""));
        ui->setmmdbButton->setEnabled(true);
        ui->saveConfigButton->setEnabled(true);
        ui->menuBar->setDisabled(true);
        ui->autosavecheckBox->setChecked(true);
        ui->windowsDriveLtrEdit->setText(tr(""));
        ui->setgetplaylistLabel->setText(tr("Select playlist for adding tracks"));
        ui->mainQTabWidget->setCurrentIndex(1);
        ui->setlibrarylabel->setText(tr("Set the home directory (top level) of the music library and store in user settings."));
        ui->setmmpllabel->setText(tr("Select the shared Windows directory where you store your m3u playlists."));
        ui->setmmdblabel->setText(tr("Select the shared Windows directory where you stored the MediaMonkey database (MM.DB) backup file."));
        ui->instructionlabel->setText(tr("ArchSimian Setup: \n(1) Identify the location where your music library is stored  \n(2) Set the "
                                         "location where you did backups for your M3U playlist \n(3) Set the location where you did backup "
                                         "of the MM.DB file \n(4) Check whether to enable new tracks \n(5) Check whether to enable album-level "
                                         "variety. \n(6) Enter the Windows drive letter of the music library. \n"
                                         "(7) Close (which saves the locations and settings) and restart the program."));
    }

    // Step 3. Determine if Archsimian songs table exists: If user configuration exists and MM4 data exists (s_bool_IsUserConfigSet and s_bool_MMdbExist are true),
    // determine if cleanlib.dsv songs table (cleanLibFile) exists in AS, function doesFileExist (const std::string& name)  (dir paths corrected,
    // imported from MM.DB) (sets s_bool_CleanLibExist)
    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist)) {
        bool tmpbool{true};
        tmpbool = doesFileExist(cleanLibFile);
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 3. tmpbool indicating cleanLibFile file: " << cleanLibFile << "  exists result: " << tmpbool << std::endl;
        if (tmpbool){ // check that file is not empty
            //Check whether the songs table currently has any data in it
            std::streampos cleanLibFilesize;
            char * memblock;
            std:: ifstream file (cleanLibFile, std::ios::in|std::ios::binary|std::ios::ate);
            if (file.is_open())
            {
                cleanLibFilesize = file.tellg();
                if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 3. cleanLibFilesize result: " << cleanLibFilesize << std::endl;
                memblock = new char [static_cast<unsigned long>(cleanLibFilesize)];
                file.seekg (0, std::ios::beg);
                file.read (memblock, cleanLibFilesize);
                file.close();
                delete[] memblock;
            }
            if (cleanLibFilesize != 0) {s_bool_CleanLibExist = true;}
        }
    }
    if ((Constants::kVerbose)&&(s_bool_IsUserConfigSet)) std::cout << "Archsimian.cpp: Step 3. Does CleanLibFile exist. s_bool_CleanLibExist result: "
                                                                                  << s_bool_CleanLibExist << std::endl;

    // 4. Determine if MM.DB was recently updated: s_bool_MMdbUpdated is set by comparing MM.DB file date
    // to CleanLib (songs table) file date. If the MM.DB file date is newer (greater) than the CleanLib file date
    // will need to be updated.

    if (s_bool_IsUserConfigSet) {
        s_bool_MMdbUpdated = recentlyUpdated(s_mmBackupDBDir);
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 4. Is the MM.DB file date newer (greater) than the CleanLibFile date."
                                                     " s_bool_MMdbUpdated result: "<< s_bool_MMdbUpdated << std::endl;
        // set UI labels if MM.DB was recently updated
        if (s_bool_MMdbUpdated) // bool s_bool_MMdbUpdated: 1 means refresh DB, 0 means skip
            // If newer is true, removeAppData ratedabbr.txt, ratedabbr2.txt, artistsadj.txt, playlistposlist.txt, artistexcludes.txt, and cleanedplaylist.txt files
        {
            QMessageBox msgBox;
            msgBox.setText("ArchSimian Database Update: A new MediaMonkey database backup has been "
                                                                 "identified, so updating ArchSimian may take some time."
                                                                 " The program window will appear once the update has been completed.");
            msgBox.exec();
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

    // Step 5. If user configuration and MM4 data exist, but the songs table does not, import songs table (from MM.DB to libtable.dsv to cleanlib.dsv)
    // into Archsimian: If user configuration and MM4 data exist, but the songs table does not (bool_IsUserConfigSet, s_bool_MMdbExist are true,
    // s_bool_CleanLibExist is false), import songs table into AS, by running writeSQLFile() function, which creates the temporary basic table file
    // libtable.dsv; then run the getLibrary() function, which creates the refined table file cleanlib.dsv The getLibrary() function completes the
    // following refinements: (a) corrects the directory paths to Linux, (b) adds random lastplayed dates for rated or "new-need-to-be-rated"
    // tracks that have no play history, (c) creates rating codes for any blank values found in GroupDesc col, using POPM values,
    // and (d) creates Artist codes (using col 1) and places the code in Custom2 if Custom2 is blank. Then set s_bool_CleanLibExist to true, rechecking,
    // run doesFileExist (const std::string& name) function. After verifying  cleanlib.dsv exists, remove temporary basic table file
    // libtable.dsv Evaluates s_bool_CleanLibExist for existence of cleanlib.dsv (cleanLibFile)

    if ((Constants::kVerbose)&&(!s_bool_MMdbUpdated)&&(s_bool_CleanLibExist)&&(s_bool_IsUserConfigSet)){
        std::cout << "Archsimian.cpp: Step 5. CleanLib file exists and MM.DB was not recently updated. Skip to Step 6."<< std::endl;}

    if (((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (!s_bool_CleanLibExist)) || (s_bool_MMdbUpdated)) {
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 5. User configuration and MM.DB exists, but the songs table does not, or MM.DB was"
                                                     "recently updated. Importing songs table (create CleanLib) into Archsimian from MM.DB..." <<std::endl;
        writeSQLFile(); // Create a SQL file with instructions to extract the Songs table from the MediaMonkey database
        pid_t c_pid;// Create a fork object; child to get database table into a dsv file, then child to open that table only
        // after it finishes getting written, not before.
        c_pid = fork(); // Run fork function
        int status; // For status of pid process
        if( c_pid == 0 ){ // Child process: Execute SQL file to get the Songs table from MediaMonkey database and save it as libtable.dsv.
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
            getLibrary(s_musiclibrarydirname,&s_musiclibshortened, &s_windowstopfolder); // get songs table from MM.DB
            removeSQLFile();
            m_prefs.s_windowstopfolder = s_windowstopfolder;
            m_prefs.s_musiclibshortened = s_musiclibshortened;
            if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 5: s_musiclibshortened: " << s_musiclibshortened.toStdString()<< " and "
                                                                                     "s_windowstopfolder: "<<s_windowstopfolder.toStdString() << std::endl;
            if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 5: s_musiclibshortened: " << s_musiclibshortened.toStdString()<< " and s_windowstopfolder: "<<s_windowstopfolder.toStdString() << std::endl;
            s_bool_CleanLibExist = doesFileExist (cleanLibFile);
        }
    }
    //Set bool for 'does top level windows folder exist' from user config
    if (s_windowstopfolder.toStdString()== ""){s_topLevelFolderExists = false;}
    else{s_topLevelFolderExists = true;}
    if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 5 s_topLevelFolderExists = "<<s_topLevelFolderExists<< std::endl;
    if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 5 completed." << std::endl;


    if (s_bool_CleanLibExist) {removeAppData ("libtable.dsv");}
    else {
        std::cout << "Archsimian.cpp: Step 6. Unable to create cleanLibFile, cleanlib.dsv." << std::endl;
        s_bool_CleanLibExist = false;
    }

    // Step 6. If user configuration exists, MM.DB exists and songs table exists, process/update statistics: If user configuration exists, MM4 data exists,
    // songs table exists (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist are all true), run function to process/update statistics getDBStats()
    if ((Constants::kVerbose) && (s_bool_IsUserConfigSet)&& (s_bool_MMdbExist) && (s_bool_CleanLibExist))std::cout
            << "Archsimian.cpp: Step 6. User configuration exists, MM.DB exists and songs table exists. Processing database statistics." << std::endl;
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
        // Total time in hours per rating code
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
        s_listeningRate = ((s_SQL10TotTimeListened/Constants::kDaysPerListeningPeriod)*Constants::kSumOfTheYearDigitsPeriod1) +
                ((s_SQL20TotTimeListened/Constants::kDaysPerListeningPeriod)*Constants::kSumOfTheYearDigitsPeriod2)
                + ((s_SQL30TotTimeListened/Constants::kDaysPerListeningPeriod)*Constants::kSumOfTheYearDigitsPeriod3) +
                ((s_SQL40TotTimeListened/Constants::kDaysPerListeningPeriod)*Constants::kSumOfTheYearDigitsPeriod4) +
                ((s_SQL50TotTimeListened/Constants::kDaysPerListeningPeriod)*Constants::kSumOfTheYearDigitsPeriod5) +
                ((s_SQL60TotTimeListened/Constants::kDaysPerListeningPeriod)*Constants::kSumOfTheYearDigitsPeriod6);
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
        s_DaysBeforeRepeatCode3 = s_yrsTillRepeatCode3 / Constants::kFractionOneDay; // fraction for one day (1/365)
        s_totalRatedTime = s_rCode1TotTime + s_rCode3TotTime + s_rCode4TotTime + s_rCode5TotTime + s_rCode6TotTime +
                s_rCode7TotTime + s_rCode8TotTime;
        s_AvgMinsPerSong = (s_totalRatedTime / s_totalRatedQty) * Constants::kSecondsToMins;
        s_avgListeningRateInMins = s_listeningRate * Constants::kSecondsToMins;
        s_SequentialTrackLimit = int((s_avgListeningRateInMins / s_AvgMinsPerSong) * s_DaysBeforeRepeatCode3);
        s_totalAdjRatedQty = (s_yrsTillRepeatCode3factor * s_rCode3TotTrackQty)+(s_yrsTillRepeatCode4factor * s_rCode4TotTrackQty)
                + (s_yrsTillRepeatCode5factor * s_rCode5TotTrackQty) +(s_yrsTillRepeatCode6factor * s_rCode6TotTrackQty)
                +(s_yrsTillRepeatCode7factor * s_rCode7TotTrackQty) + (s_yrsTillRepeatCode8factor * s_rCode8TotTrackQty);

        selTrackLimitCodeTotTrackQty = std::min({s_rCode3TotTrackQty, s_rCode4TotTrackQty, s_rCode5TotTrackQty, s_rCode6TotTrackQty,
                                                 s_rCode7TotTrackQty, s_rCode8TotTrackQty},comp);
        if (selTrackLimitCodeTotTrackQty == s_rCode3TotTrackQty)
        {
            selectedTrackLimitCode = 3;
            selTrackLimitCodeRatingRatio = s_ratingRatio3;
        }
        playlistTrackLimitCodeQty = (int (selTrackLimitCodeRatingRatio * selTrackLimitCodeTotTrackQty * trackLimitPercentage));
        if (Constants::kVerbose) {
            std::cout << "selTrackLimitCodeTotTrackQty is: "<< selTrackLimitCodeTotTrackQty << std::endl;
            std::cout << "selectedTrackLimitCode is: "<< selectedTrackLimitCode << std::endl;
            std::cout << "selTrackLimitCodeRatingRatio is: "<< selTrackLimitCodeRatingRatio << std::endl;
            std::cout << "trackLimitPercentage is: "<< trackLimitPercentage << std::endl;            
            std::cout << "playlistTrackLimitCodeQty is: "<< playlistTrackLimitCodeQty << std::endl;
        }
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
            std::cout << "Archsimian.cpp: Step 6. Something went wrong processing the function getDBStats." << std::endl;
        }
    }

    // Step 7a. If user configuration exists, MM.DB exists, songs table exists, statistics are processed, and
    // MM.DB was not recently updated, check for state of s_bool_artistsadjExist (artistsadj.txt).
    // If file is missing or empty, create file with artist statistics
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
            else std::cout << "Archsimian.cpp: Step 7. There was a problem opening artistsadj.txt" << std::endl;

            if (artsistAdjsize != 0) {s_bool_artistsadjExist = true;// file artistsadj.txt exists and is greater in size than zero, set to true
                // If MM.DB not recently updated and artistsadj.txt does not need to be updated, check if ratedabbr.txt exists
                // If it does set s_bool_RatedAbbrExist to true.
                if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 7. MM.DB not recently updated and artistsadj.txt does not need to be updated. "
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
                    else std::cout << "Archsimian.cpp: Step 7. There was a problem opening ratedabbr.txt" << std::endl;
                    if (ratedabbrsize != 0) {
                        s_bool_RatedAbbrExist = true;
                        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 7. Set s_bool_RatedAbbrExist = true." << std::endl;}
                    }
                }
            }
            if (artsistAdjsize == 0) {s_bool_artistsadjExist = false;}// file exists but size is zero, set to false
        }
        if (!tmpbool){s_bool_artistsadjExist = false;} // file does not exist, set bool to false

        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 7. MM.DB not recently updated. Verifying artistsadj.txt exists and is not zero. "
                                                     "s_bool_artistsadjExist result: "<< s_bool_artistsadjExist << std::endl;
        if (!s_bool_artistsadjExist){
            getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                                   &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                                   &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                                   &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);
            s_bool_artistsadjExist = doesFileExist (appDataPathstr.toStdString()+"/artistsadj.txt");
            s_bool_RatedAbbrExist = false;
            if (!s_bool_artistsadjExist)  {std::cout << "Archsimian.cpp: Step 7(a) Something went wrong at the function getArtistAdjustedCount. artistsadj.txt not created." << std::endl;}
        }
    }

    // Step 7b. If user configuration exists, MM.DB exists, songs table exists, statistics are processed, and
    // MM.DB was recently updated, create file with artist statistics

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist) &&
            (s_bool_dbStatsCalculated) && (s_bool_MMdbUpdated)) {
        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 7. MM.DB was recently updated. Processing artist statistics..." << std::endl;}
        getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                               &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                               &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                               &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);
        s_bool_RatedAbbrExist = false;
        s_bool_artistsadjExist = doesFileExist (appDataPathstr.toStdString()+"/artistsadj.txt");
        if (!s_bool_artistsadjExist)  {std::cout << "Archsimian.cpp: Step 7(b). Something went wrong at the function getArtistAdjustedCount. artistsadj.txt not created." << std::endl;}
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
        if (!s_bool_RatedAbbrExist)  {std::cout << "Archsimian.cpp: Step 8. Something went wrong at the function getSubset(). ratedabbr.txt not created." << std::endl;}
        if ((s_bool_RatedAbbrExist) && (Constants::kVerbose)){std::cout << "Archsimian.cpp: Step 8. ratedabbr.txt was created." << std::endl;}
    }
    if ((Constants::kVerbose) && (s_bool_RatedAbbrExist) && (s_bool_IsUserConfigSet)){std::cout
                << "Archsimian.cpp: Step 8. MM.DB and artist.adj not recently updated. ratedabbr.txt not updated." << std::endl;}

    // 9. Set playlist exists to false always to force reloading of playlist every time the program starts. Also set status of playlist selection from configuration.

    s_bool_PlaylistExist = false;
    s_bool_PlaylistSelected = true;
    if (s_defaultPlaylist == ""){
        s_bool_PlaylistSelected = false;
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 9. Playlist set to 'not exist'. Playlist not selected." << std::endl;}
    }
    else{if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 9. Playlist set to 'not exist'. Default playlist identified." << std::endl;}
        s_topLevelFolderExists = true; // check temporary
    }
    // 10. If a playlist was identified in the user config (step 9), generate the playlist file: If user configuration
    // exists, MM4 data exists, songs table exists (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist are all true), and playlist from user config exists
    // (s_bool_PlaylistSelected is true), run function to generate cleaned playlist file getPlaylist()
    // then set s_bool_PlaylistExist to true, rechecking, run doesFileExist (const std::string& name) function. Evaluates s_bool_PlaylistExist and sets to true
    // (after running getPlaylist) if initially false

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist) && (s_bool_PlaylistSelected) && (s_topLevelFolderExists)){
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 10. Regenerating 'cleanedplaylist' file for editing using default playlist identified in user config." << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 10. Prior to running getPlaylist, here are the values for the five "
                                              "inputs: "<< s_bool_IsUserConfigSet<<", \n"<< s_defaultPlaylist.toStdString()<<", \n"<< s_musiclibrarydirname.toStdString()<<
                                              ", \n"<<s_musiclibshortened.toStdString()<<", and \n"<<s_topLevelFolderExists<< std::endl;}
        getPlaylist(s_defaultPlaylist, s_musiclibrarydirname, s_musiclibshortened, s_topLevelFolderExists);
        s_bool_PlaylistExist = doesFileExist (appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        QFileInfo fi(s_defaultPlaylist);
        QString justname = fi.fileName();
        QMainWindow::setWindowTitle("ArchSimian - "+justname);
        if (!s_bool_PlaylistExist) {std::cout << "Archsimian.cpp: Step 10. Something went wrong at the function getPlaylist." << std::endl;}
    }
    //if ((Constants::kVerbose) && (s_bool_PlaylistExist) && (s_bool_IsUserConfigSet)){std::cout << "Archsimian.cpp: Step 10. Playlist exists and was not updated." << std::endl;}

    // 10a. If a playlist was not identified in the user config, adjust the UI accordingly

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist) && (!s_bool_PlaylistSelected) && (!s_bool_PlaylistExist)){

    ui->setgetplaylistLabel->setText("No playlist selected");
    ui->addsongsButton->setEnabled(false);
    ui->addsongsLabel->setText(tr(""));
    ui->viewplaylistButton->setDisabled(true);
    ui->viewplaylistLabel->setText(tr("No playlist selected"));    
    QMainWindow::setWindowTitle("ArchSimian - No playlist selected");
    }

    // NOTE: functions used in the next three steps (11-15) will later be reused when adding tracks to
    // playlist - here is to get the initial values if a playlist exists

    //11. If playlist exists, calculate the playlist size: If cleaned playlist exists (s_bool_PlaylistExist is true), obtain playlist size
    // using function cstyleStringCount(),  s_playlistSize = cstyleStringCount(cleanedPlaylist); And, get the Windows drive letter.
    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet)) {
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        if (s_playlistSize < 2) {
            s_playlistSize = 0;
            ui->viewplaylistButton->setDisabled(true);
            //ui->addsongsButton->setEnabled(true);
            //ui->addsongsLabel->setText(tr("tracks to selected playlist."));
            ui->viewplaylistLabel->setText(tr("Current playlist is empty"));
            QMainWindow::setWindowTitle("ArchSimian - No playlist selected");
        }
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 11. Playlist size is: "<< s_playlistSize << std::endl;}        
        //s_winDriveLtr = m_prefs.s_WindowsDriveLetter;
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 11. Windows drive letter loaded from user configuration "
                                              "as: "<< s_winDriveLtr.toStdString() << std::endl;}
    }

    // 12. If playlist exists, obtain the historical count (in addition to the playlist count) up to the sequential track limit:
    // If cleaned playlist exists (s_bool_PlaylistExist is true), obtain the historical count (in addition to the playlist count) up to the
    // sequential track limit. A variable is needed (which later will be used to obtain additional play history outside of
    // playlist, as part of a later function to make a new track selection), using the variable s_histCount. The value is
    // calculated [ can be modified to use the function  to added function void getHistCount(&s_SequentialTrackLimit,&s_playlistSize),
    // or just: s_histCount = long(s_SequentialTrackLimit) – long(s_playlistSize); this uses both playlist size from 10
    // and SequentialTrackLimit obtained with data from function getDBStats()
    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet)) {
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        s_histCount = int(s_SequentialTrackLimit - s_playlistSize);
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 12. s_histCount is: "<< s_histCount << std::endl;}
    }

    //13. If playlist exists, artist statistics are processed, and modified database exists, create/update excluded artists
    // list. Creates temporary database (ratedabbr2.txt) with playlist position numbers for use in subsequent functions,
    //ratingCodeSelected and selectTrack: If cleaned playlist exists (s_bool_PlaylistExist is true), and artistsadj.txt exists (s_bool_artistsadjExist is true) and modified database exists
    // (s_bool_RatedAbbrExist), run function getExcludedArtists() to create/update excluded artists list using vectors
    // read in from the following files: cleanlib.dsv, artistsadj.txt, and cleanedplaylist.txt. Writes artistexcludes.txt.

    if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 13. Processing artist stats and excluded artists list. Creating temporary "
                                          "database (ratedabbr2.txt) with playlist position numbers for use in subsequent "
                                          "functions, ratingCodeSelected and selectTrack."<< s_histCount << std::endl;}
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
        ui->factor3horizontalSlider->setMinimum(Constants::kRatingCode3MinDays);
        ui->factor3horizontalSlider->setMaximum(Constants::kRatingCode3MaxDays);
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
        if (m_prefs.s_disableNotificationAddTracks){
            ui->disablenotecheckBox->setChecked(true);
        }
        if (!m_prefs.s_disableNotificationAddTracks){
            ui->disablenotecheckBox->setChecked(false);
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
      This generates the file artistalbmexcls.txt (see albumexcludes project) */

    if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 14. If user selects bool for s_includeAlbumVariety, run function buildAlbumExclLibrary."<< s_includeAlbumVariety << std::endl;}

    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet) && (s_includeAlbumVariety))
    {
        buildAlbumExclLibrary(s_minalbums, s_mintrackseach, s_mintracks);
        ui->albumsTab->setEnabled(true);
    }
    //15. Sets the initial playlist size limit to restrict how many tracks can be added to the playlist
    if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. Run setPlaylistLimitCount to set the initial count for playlistLimitCount." << std::endl;}

    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet))
    {
       // Run setPlaylistLimitCount whenever an existing playlist is opened, Set the initial count for playlistLimitCount:

        // NEW If a playlist exists, count the number of 'limit' tracks (s_playlistActualCntSelCode)in the playlist at initial launch

        setPlaylistLimitCount (selectedTrackLimitCode, &s_playlistActualCntSelCode);

        // Calculate the maximum tracks that can be added to the playlist, based on current playlist size, the
        // frequency of adding code 3 (or other code) to the playlist, and the number of code 3 tracks currently
        // in the playlist.

        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 15. playlistTrackLimitCodeQty before s_MaxAvailableToAdd calc is: "<< playlistTrackLimitCodeQty << std::endl;
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 15. s_playlistActualCntSelCode (actual 3s in playlist) "
                                              "before s_MaxAvailableToAdd calc is: "<< s_playlistActualCntSelCode << std::endl;
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. s_playlistSize before s_MaxAvailableToAdd calc is: "<< s_playlistSize << std::endl;}

        // Count the initial s_MaxAvailableToAdd for the playlist at initial launch
        s_MaxAvailStaticCast = static_cast<double>(s_playlistSize) / static_cast<double>(s_playlistActualCntSelCode);
        s_MaxAvailableToAdd = int (static_cast<double>(playlistTrackLimitCodeQty - s_playlistActualCntSelCode) * (s_MaxAvailStaticCast));
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. playlistTrackLimitCodeQty - s_playlistActualCntSelCode is: "<< playlistTrackLimitCodeQty - s_playlistActualCntSelCode << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. s_MaxAvailStaticCast is: "<< s_MaxAvailStaticCast << std::endl;}
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 15. s_MaxAvailableToAdd after variable check is: "<< s_MaxAvailableToAdd << std::endl;
        // If the playlist is small, force a baseline max of 30
        if (s_playlistSize < 30){
            s_MaxAvailableToAdd = 50;
            ui->addtrksspinBox->setValue(20);
            ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: 50"));
        }
        // Determine if playlist is already full at program launch by comparing s_playlistActualCntSelCode to playlistTrackLimitCodeQty, then set bool
        if (s_playlistSize > 29){
            if (s_playlistActualCntSelCode > playlistTrackLimitCodeQty){
                playlistFull = true;
                ui->addsongsButton->setEnabled(false);
                ui->addsongsLabel->setText(tr("Playlist is at maximum size."));
            }
            else {
                ui->addtrksspinBox->setMaximum(s_MaxAvailableToAdd);
                if (s_MaxAvailableToAdd > 9) { ui->addtrksspinBox->setValue(10);}
                if (s_MaxAvailableToAdd < 10) {ui->addtrksspinBox->setValue(s_MaxAvailableToAdd-1);}
                ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd,'g', 3));
            }
        }
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 15. s_MaxAvailableToAdd at program launch is: "<< s_MaxAvailableToAdd << std::endl;
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 15. Playlist full status is: "<< playlistFull << std::endl;
        }
    // End setup of UI
}

// Functions available from within the UI

void ArchSimian::on_addsongsButton_released(){
    if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Starting addSongs function." << std::endl;
    // First set messages and feedback to user during process
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    int numTracks = ui->addtrksspinBox->value(); // Sets the number of tracks the user selected to add (numtracks)
    // Reduce s_MaxAvailableToAdd quantity by number of tracks to be added
    if (s_MaxAvailableToAdd > 0){
        s_MaxAvailableToAdd = s_MaxAvailableToAdd - numTracks;
    }
    // If tracks added makes s_MaxAvailableToAdd = 0, then dim the button and change label
    if (s_MaxAvailableToAdd == 0){
        playlistFull = true;
        ui->addsongsButton->setEnabled(false);
        ui->addsongsLabel->setText(tr("Playlist is at maximum size."));
    }

    if (numTracks > Constants::kNotifyTrackThreshold) {
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
    if (s_playlistSize == 0) {
        std::ofstream ofs; //open the cleanedplaylist file for writing with the truncate option to delete the content.
        ofs.open(appDataPathstr.toStdString()+"/cleanedplaylist.txt", std::ofstream::out | std::ofstream::trunc);
        ofs.close();
    }

    std::ofstream songtext(appDataPathstr.toStdString()+"/songtext.txt",std::ios::app); // output file append mode for writing final song selections (UI display)
    // Second, determine the rating for the track selection
    if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Running ratingCodeSelected function before loop."<< std::endl;
    s_ratingNextTrack = ratingCodeSelected(s_ratingRatio3,s_ratingRatio4,s_ratingRatio5,s_ratingRatio6,
                                           s_ratingRatio7,s_ratingRatio8);
     if (s_playlistSize == 0) {s_ratingNextTrack = 6;}

    if (Constants::kVerbose) std::cout <<"on_addsongsButton_released: ratingCodeSelected function before loop completed. Result is: "<< s_ratingNextTrack <<
                                                ". Now starting loop (913) to select tracks and add them to playlist..." <<std::endl;
    // Third, start loop for the number of tracks the user selected to add (numtracks)
    for (int i=0; i < numTracks; i++){
        if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Top of Loop (914). Count: " <<i<<". Adding track "<< i + 1<<"." <<std::endl;
        s_uniqueCode1ArtistCount = 0;
        s_code1PlaylistCount = 0;
        s_lowestCode1Pos = Constants::kMaxLowestCode1Pos;
        s_selectedCode1Path = "";
        s_selectedTrackPath = "";
        if ((s_includeNewTracks) && (s_playlistSize > 0)){
            // If user is including new tracks, determine if a code 1 track should be added for this particular selection
            code1stats(&s_uniqueCode1ArtistCount,&s_code1PlaylistCount, &s_lowestCode1Pos, &s_artistLastCode1);// Retrieve rating code 1 stats
            // Use stats to check that all code 1 tracks are not already in the playlist, and the repeat frequency is met
            if ((s_code1PlaylistCount < s_rCode1TotTrackQty) && ((s_lowestCode1Pos + 1) > s_repeatFreqForCode1)){
                getNewTrack(s_artistLastCode1, &s_selectedCode1Path); // Get rating code 1 track selection if criteria is met
                s_selectedTrackPath = s_selectedCode1Path; // set the track selection to the code 1 selection
                if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Rating code 1 applies to current track selection: " << s_selectedTrackPath << std::endl <<
                                                             "Code 1 track added to playlist."<< std::endl;
            }
        }
        else {s_selectedCode1Path = "";}

        // If selection criteria for rating code 1 is not met, return empty string
        if (!s_selectedCode1Path.empty()) {
            s_ratingNextTrack = 1;} // If string is not empty, set rating for next track as code 1
        if ((!s_includeNewTracks)||(s_ratingNextTrack != 1)) { // If user excluded new tracks, or set rating code is not 1, do normal selection
            if ((Constants::kVerbose)&&(!s_includeNewTracks)) std::cout << "User excluding new tracks. Check whether user selected album variety " << std::endl;
            if (s_includeAlbumVariety){ // If not 1, and user has selected album variety, get album ID stats                
                if (Constants::kVerbose) std::cout << "on_addsongsButton_released: User selected album variety. Getting functions getTrimArtAlbmList and getAlbumIDs." << std::endl;
                getTrimArtAlbmList();
                getAlbumIDs();
            }
            if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Now selecting track for non-code-1 track selection (function selectTrack)." << std::endl;
            try {
            selectTrack(s_ratingNextTrack,&s_selectedTrackPath,s_includeAlbumVariety); // Select track if not a code 1 selection
            }
            catch (const std::bad_alloc& exception) {
                std::cerr << "on_addsongsButton_released: bad_alloc detected: Maximum playlist length has been reached. Exiting program." << exception.what();
                i = numTracks;
                QMessageBox msgBox;
                QString msgboxtxt = "on_addsongsButton_released: Out of memory error (bad_alloc):failed during attempt to add tracks. Likely reason: "
                                    "Not enough available tracks found. Lower track limit buffer variable and restart.";
                msgBox.setText(msgboxtxt);
                msgBox.exec();                
                removeAppData("cleanlib.dsv");
                removeAppData("playlistposlist.txt");
                qApp->quit(); //Exit program
              }
        }
        // Collect and collate 'track selected' info for (UI display of) final song selections
        std::string shortselectedTrackPath;
        shortselectedTrackPath = s_selectedTrackPath;
        std::string key1 ("/");
        std::string key2 ("_");
        // Next, determine how many alphanumeric chars there are in the windowstopfolder name

        char *array_point;
        char c1;
        unsigned long count=0, alp=0, digt=0, oth=0;
        char string_array[100];
        strcpy(string_array, s_musiclibrarydirname.toStdString().c_str());
        for(array_point=string_array;*array_point!='\0';array_point++)
        {
            c1=*array_point;
            count++;
            if (isalpha(c1))
            {
                alp++;
            }
            else
                if (isdigit(c1))
                {
                    digt++;
                }
                else
                {
                    oth++;
                }
        }
        shortselectedTrackPath.erase(0,count);
        std::size_t found = shortselectedTrackPath.rfind(key1);
        std::size_t found1 = shortselectedTrackPath.rfind(key2);
        if (found!=std::string::npos){shortselectedTrackPath.replace (found,key1.length(),", ");}
        if (found1!=std::string::npos){shortselectedTrackPath.replace (found,key2.length(),"");}
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        songtext << s_playlistSize<<". "<< shortselectedTrackPath <<'\n'; // adds the playlist pos number and track to the text display file
        if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Track has now been added to the playlist. New playlist length is: " << s_playlistSize << " tracks." << std::endl;
        // Calculate excluded artists and get rating for next track selection (accounting for track just added)
        s_histCount = long(s_SequentialTrackLimit) - long(s_playlistSize); // Recalc historical count (outside playlist count) up to sequential track limit
        getExcludedArtists(s_playlistSize); // Recalc excluded artists
        if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Running ratingCodeSelected function in loop."<< std::endl;
        s_ratingNextTrack = ratingCodeSelected(s_ratingRatio3,s_ratingRatio4,s_ratingRatio5,s_ratingRatio6,
                                               s_ratingRatio7,s_ratingRatio8); // Recalc rating selection
        if (Constants::kVerbose) std::cout<< "on_addsongsButton_released: ratingCodeSelected function in loop completed. Result: " << s_ratingNextTrack << ". Count "
                                                            "at end (1006) is now: "<< i<< std::endl;
        if (Constants::kVerbose) std::cout<< "on_addsongsButton_released: *****************************************************************" << std::endl;
        if (Constants::kVerbose) std::cout<< "on_addsongsButton_released: *************   Added track "<< i + 1<<".   ********************" << std::endl;
        if (Constants::kVerbose) std::cout<< "on_addsongsButton_released: *****************************************************************" << std::endl;
        // If track selected is a 3 (or other selectedTrackLimitCode), then increase the count
        if (s_ratingNextTrack == selectedTrackLimitCode){++s_playlistActualCntSelCode;}

    }
     //After all tracks have been processed, update UI with information to user about tracks added to playlist
    // First, update playlist limit and recalc s_MaxAvailableToAdd

    if (Constants::kVerbose) std::cout << "on_addsongsButton_released: s_playlistActualCntSelCode (actual 3s in playlist) "
                                                  "(after all tracks have been processed) is: "<< s_playlistActualCntSelCode << std::endl;
    if (Constants::kVerbose){std::cout << "on_addsongsButton_released: s_playlistSize before s_MaxAvailableToAdd calc is: "<< s_playlistSize << std::endl;}

    s_MaxAvailStaticCast = static_cast<double>(s_playlistSize) / static_cast<double>(s_playlistActualCntSelCode);
    s_MaxAvailableToAdd = int (static_cast<double>(playlistTrackLimitCodeQty - s_playlistActualCntSelCode) * (s_MaxAvailStaticCast));
    if (Constants::kVerbose) std::cout << "on_addsongsButton_released: s_MaxAvailableToAdd (after all tracks have been processed) is: "<< s_MaxAvailableToAdd << std::endl;
    if (Constants::kVerbose){std::cout << "on_addsongsButton_released: playlistTrackLimitCodeQty - s_playlistActualCntSelCode is: "<< playlistTrackLimitCodeQty - s_playlistActualCntSelCode << std::endl;}
    if (Constants::kVerbose){std::cout << "on_addsongsButton_released: s_MaxAvailStaticCast is: "<< s_MaxAvailStaticCast << std::endl;}
    if (Constants::kVerbose) std::cout << "on_addsongsButton_released: s_MaxAvailableToAdd after variable check is: "<< s_MaxAvailableToAdd << std::endl;
    if (s_MaxAvailableToAdd < 2){
        playlistFull = true;
        ui->addsongsButton->setEnabled(false);
        ui->addsongsLabel->setText(tr("Playlist is at maximum size."));
    }
    else {
        // Second, determine if playlist is now full after tracks add by comparing s_playlistActualCntSelCode to playlistTrackLimitCodeQty, then set bool
        ui->addtrksspinBox->setMaximum(s_MaxAvailableToAdd);
        if (s_MaxAvailableToAdd > 9) { ui->addtrksspinBox->setValue(10);}
        if (s_MaxAvailableToAdd < 10) {ui->addtrksspinBox->setValue(s_MaxAvailableToAdd-1);}
        ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd-1,'g', 3));
    }
    // If the playlist is small, force a baseline max of 50
    if (s_playlistSize < 30){
        s_MaxAvailableToAdd = 50;
    }
    if (s_includeNewTracks){  // If user is including new tracks, determine if a code 1 track should be added for this particular selection
        ui->newtracksqtyLabel->setText(tr("New tracks qty not in playlist: ") + QString::number(s_rCode1TotTrackQty - s_code1PlaylistCount));
    }
    songtext.close();
    ui->currentplsizeLabel->setText(tr("Current playlist size is ") + QString::number(s_playlistSize)+tr(" tracks, "));
    ui->playlistdaysLabel->setText(tr("and playlist length in listening days is ") +
                                          QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
    ui->statusBar->showMessage("Added " + QString::number(numTracks) + " tracks to playlist",4000);
    ui->addtrksspinBox->setMaximum(s_MaxAvailableToAdd);
    if (s_MaxAvailableToAdd > 9) { ui->addtrksspinBox->setValue(10);}
    if (s_MaxAvailableToAdd < 10) {ui->addtrksspinBox->setValue(s_MaxAvailableToAdd-1);}
    QFile songtext1(appDataPathstr+"/songtext.txt");
    if(!songtext1.open(QIODevice::ReadOnly))
        QMessageBox::information(nullptr,"info",songtext1.errorString());
    QTextStream in(&songtext1);
    ui->songsaddtextBrowser->setText(in.readAll());
    if (s_MaxAvailableToAdd > 0) {ui->addsongsButton->setEnabled(true);}
    ui->viewplaylistButton->setDisabled(false);
    ui->viewplaylistLabel->setText(tr("View currently selected playlist"));
    QFileInfo fi(s_defaultPlaylist);
    QString justname = fi.fileName();
    QMainWindow::setWindowTitle("ArchSimian - "+justname);
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
    ui->saveConfigButton->setEnabled(true);
    m_prefs.musicLibraryDir = s_musiclibrarydirname;
    ui->setmmplButton->setEnabled(true);    
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
    ui->saveConfigButton->setEnabled(true);
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
    ui->saveConfigButton->setEnabled(true);
    m_prefs.mmBackupDBDir = mmbackupdbdirname;
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
}

void ArchSimian::on_repeatFreq1SpinBox_valueChanged(int myvalue)
{
    m_prefs.repeatFreqCode1 = myvalue;
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
    m_prefs.s_WindowsDriveLetter = settings.value("s_winDriveLtr",Constants::kUserDefaultWindowsDriveLetter).toString();
    m_prefs.s_minalbums = settings.value("s_minalbums", Constants::kUserDefaultMinalbums).toInt();
    m_prefs.s_mintrackseach = settings.value("s_mintrackseach", Constants::kUserDefaultMintrackseach).toInt();
    m_prefs.s_mintracks = settings.value("s_mintracks", Constants::kUserDefaultMintracks).toInt();
    m_prefs.s_windowstopfolder = settings.value("s_windowstopfolder",Constants::kWindowsTopFolder).toString();
    m_prefs.s_musiclibshortened = settings.value("s_musiclibshortened",Constants::kMusicLibShortened).toString();
//    m_prefs.s_playlistActualCntSelCode = settings.value("s_playlistActualCntSelCode", Constants::kPlaylistActualCntSelCode).toInt();
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
    settings.setValue("s_winDriveLtr",m_prefs.s_WindowsDriveLetter);
    settings.setValue("s_windowstopfolder",m_prefs.s_windowstopfolder);
    settings.setValue("s_musiclibshortened",m_prefs.s_musiclibshortened);
//    settings.setValue("s_playlistActualCntSelCode", m_prefs.s_playlistActualCntSelCode);
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
    if (!static_cast<bool>(ui->InclNewcheckbox->checkState())){
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

void ArchSimian::on_actionExport_Playlist_triggered()
{
    int s_musicdirlength{};
    s_musicdirlength = musicLibraryDirLen(s_musiclibrarydirname);
    s_defaultPlaylist = m_prefs.defaultPlaylist;
    playlistpath = s_defaultPlaylist.toStdString();
    if (Constants::kVerbose){std::cout << "on_actionExport_Playlist_triggered: s_defaultPlaylist save is to: "<< s_defaultPlaylist.toStdString() << std::endl;}
    exportPlaylistToWindows(s_musicdirlength, s_mmPlaylistDir,  s_winDriveLtr,  s_musiclibrarydirname, playlistpath);
    //m_prefs.s_playlistActualCntSelCode = s_playlistActualCntSelCode;
    //s_playlistActualCntSelCode = m_prefs.s_playlistActualCntSelCode;
    saveSettings();
    ui->statusBar->showMessage("Saved Archsimian-modified playlist in Windows directory format",4000);
}

void ArchSimian::on_actionExit_triggered()
{
    if (!s_noAutoSave){
        saveSettings();
        std::ofstream ofs; //open the cleanedplaylist file for writing with the truncate option to delete the content.
        ofs.open(appDataPathstr.toStdString()+"/cleanedplaylist.txt", std::ofstream::out | std::ofstream::trunc);
        ofs.close();
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: on_actionExit_triggered. cleanedplaylist should be zero now: "<< s_playlistSize << std::endl;}
       qApp->quit();
    }
    if (s_noAutoSave){
        if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Do you wish to save any changes to "
                                                      "the configuration settings before quitting?", QMessageBox::Yes | QMessageBox::No))
        {
            saveSettings();
            std::ofstream ofs; //open the cleanedplaylist file for writing with the truncate option to delete the content.
            ofs.open(appDataPathstr.toStdString()+"/cleanedplaylist.txt", std::ofstream::out | std::ofstream::trunc);
            ofs.close();
            s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
            if (Constants::kVerbose){std::cout << "Archsimian.cpp: on_actionExit_triggered. cleanedplaylist should be zero now: "<< s_playlistSize << std::endl;}
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
    ui->statusBar->showMessage("Saved user settings",4000);
}

void ArchSimian::on_actionAbout_triggered()
{
    QMessageBox::about(this,tr("ArchSimian") ,tr("\nArchSimian v.1.04"
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
    {
        // Set s_bool_PlaylistExist to false
        s_bool_PlaylistSelected = false;
        // Get playlist from user
        QFileDialog setgetplaylistdialog;
        QString selectedplaylist = QFileDialog::getOpenFileName (
                    this,
                    "Select playlist for which you will add tracks",
                    QString(s_mmPlaylistDir),//default dir for playlists
                    "playlists(.m3u) (*.m3u)");

        // Exit function if no playlist is selected
        if(selectedplaylist.isEmpty()&&selectedplaylist==""){
            return;
        }
        // Set selected as default playlist, save to settings
        if(!selectedplaylist.isEmpty()&&selectedplaylist!=""){
        m_prefs.defaultPlaylist = selectedplaylist;
        s_defaultPlaylist = m_prefs.defaultPlaylist;
        saveSettings();
        }
        QMainWindow::setWindowTitle("ArchSimian - "+s_defaultPlaylist);
        if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered: Add/change playlist.. s_defaultPlaylist is: "<< s_defaultPlaylist.toStdString() << std::endl;}
        ui->setgetplaylistLabel->setText("Selected: " + QString(selectedplaylist));
        removeAppData("cleanedplaylist.txt");
        if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered: Add/change playlist. cleanedplaylist should be zero now: "<< s_playlistSize << std::endl;}

        // Regenerate the playlist file (from step 10)
        getPlaylist(s_defaultPlaylist, s_musiclibrarydirname, s_musiclibshortened, s_topLevelFolderExists);
        s_bool_PlaylistSelected = true;
        if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered: For playlist opened, cleanedplaylist regenerated."
                                               << std::endl;}
        ui->viewplaylistButton->setDisabled(false);
        ui->viewplaylistLabel->setText(tr("View currently selected playlist"));
        QFileInfo fi(s_defaultPlaylist);
        QString justname = fi.fileName();
        QMainWindow::setWindowTitle("ArchSimian - "+justname);
        // Get playlist size (from step 11)
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        if (s_playlistSize < 2) {
            s_playlistSize = 0;
            ui->viewplaylistButton->setDisabled(true);
            ui->viewplaylistLabel->setText(tr("Current playlist is empty"));
        }
        if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered: Add/change playlist.. s_playlistSize is: "<< s_playlistSize << std::endl;}

        // Get the Windows drive letter and save to configuration file (from step 11)
        //getWindowsDriveLtr(s_defaultPlaylist, &s_winDriveLtr);
        //m_prefs.s_WindowsDriveLetter = s_winDriveLtr;

        // Recalculate historical count (from step 12)
        s_histCount = int(s_SequentialTrackLimit - s_playlistSize);
        if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered: Add/change playlist.. s_histCount is: "<< s_histCount << std::endl;}

        // Update excluded artists (from step 13) by running function getExcludedArtists() which also recreates ratedabbr2, and check code1 stats
        if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered: Processing artist stats and excluded artists list. Creating temporary "
                                              "database (ratedabbr2.txt)." << std::endl;}
        getExcludedArtists(s_playlistSize);

        if (s_includeNewTracks){  // If user is including new tracks (from step 13), determine if a code 1 track should be added for this particular selection
            code1stats(&s_uniqueCode1ArtistCount,&s_code1PlaylistCount, &s_lowestCode1Pos, &s_artistLastCode1);// Retrieve rating code 1 stats
        }
        // Recheck album excludes if enabled (from step 14)

        if (s_includeAlbumVariety)
        {
            buildAlbumExclLibrary(s_minalbums, s_mintrackseach, s_mintracks);
            ui->albumsTab->setEnabled(true);
        }
        //Sets the playlist size limit to restrict how many tracks can be added to the playlist (from step 15)
        s_playlistActualCntSelCode = 0; //First reset to zero
        setPlaylistLimitCount (selectedTrackLimitCode, &s_playlistActualCntSelCode); // Now get the count
        //s_MaxAvailStaticCast = static_cast<double>(s_playlistSize) / static_cast<double>(s_playlistActualCntSelCode);
        s_MaxAvailStaticCast = static_cast<double>(s_playlistSize) / static_cast<double>(s_playlistActualCntSelCode);
        s_MaxAvailableToAdd = int (static_cast<double>(playlistTrackLimitCodeQty - s_playlistActualCntSelCode) * (s_MaxAvailStaticCast));
        // If the playlist is small, force a baseline max of 30
        if (s_playlistSize < 30){
            s_MaxAvailableToAdd = 50;
            ui->addtrksspinBox->setValue(20);
            ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: 50"));
        }
        if (Constants::kVerbose) std::cout << "on_actionOpen_Playlist_triggered: s_MaxAvailableToAdd for playlist loaded is: "<< s_MaxAvailableToAdd << std::endl;
        if (Constants::kVerbose) std::cout << "on_actionOpen_Playlist_triggered: s_playlistActualCntSelCode for playlist loaded is: "<< s_playlistActualCntSelCode << std::endl;

        if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered. playlistTrackLimitCodeQty - "
                                              "s_playlistActualCntSelCode is: "<< playlistTrackLimitCodeQty - s_playlistActualCntSelCode << std::endl;}
         if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered. s_MaxAvailStaticCast is: "<< s_MaxAvailStaticCast << std::endl;}
         if (Constants::kVerbose) std::cout << "on_actionOpen_Playlist_triggered. s_MaxAvailableToAdd after variable check is: "<< s_MaxAvailableToAdd << std::endl;


        // Determine if playlist is already full at program launch by comparing s_playlistActualCntSelCode to playlistTrackLimitCodeQty, then set bool
         if (s_playlistSize > 29){
             // Determine if playlist is already full at program launch by comparing s_playlistActualCntSelCode to playlistTrackLimitCodeQty, then set bool
             if (s_playlistActualCntSelCode > playlistTrackLimitCodeQty){
                 playlistFull = true;
                 ui->addsongsButton->setEnabled(false);
                 ui->addsongsLabel->setText(tr("Playlist is at maximum size."));
             }
             else {
                 ui->addtrksspinBox->setMaximum(s_MaxAvailableToAdd);
                 if (s_MaxAvailableToAdd > 9) { ui->addtrksspinBox->setValue(10);}
                 if (s_MaxAvailableToAdd < 10) {ui->addtrksspinBox->setValue(s_MaxAvailableToAdd-1);}
                 ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd-1,'g', 3));
             }
         }
        // Finalize playlist loading

        ui->currentplsizeLabel->setText(tr("Current playlist size is ") + QString::number(s_playlistSize)+tr(" tracks, "));
        ui->playlistdaysLabel->setText(tr("and playlist length in listening days is ") + QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
        std::string MMdbDate = getMMdbDate(s_mmBackupDBDir);
        std::string LastTableDate = getLastTableDate();
        ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(MMdbDate)+
                                       tr(", Library date: ")+ QString::fromStdString(LastTableDate));
        ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(MMdbDate)+
                                       tr(", Library date: ")+ QString::fromStdString(LastTableDate));
        if (s_includeNewTracks){  // If user is including new tracks, determine if a code 1 track should be added for this particular selection
            s_uniqueCode1ArtistCount = 0;
            s_code1PlaylistCount = 0;
            s_lowestCode1Pos = Constants::kMaxLowestCode1Pos;
            code1stats(&s_uniqueCode1ArtistCount,&s_code1PlaylistCount, &s_lowestCode1Pos, &s_artistLastCode1);// Retrieve rating code 1 stats
            ui->newtracksqtyLabel->setText(tr("New tracks qty not in playlist: ") + QString::number(s_rCode1TotTrackQty - s_code1PlaylistCount));
        }
        ui->songsaddtextBrowser->setText("");
        ui->addsongsButton->setEnabled(true);
    }
}

void ArchSimian::on_actionNew_Playlist_triggered()
{
    // Set s_bool_PlaylistExist to false
    s_bool_PlaylistSelected = false;
    if (Constants::kVerbose){std::cout << "Archsimian.cpp: Add/change playlist. Set playlist to not exist." << std::endl;}
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    QString strFile = dialog.getSaveFileName(this, "Create New File",QString(s_mmPlaylistDir),"playlists(.m3u) (*.m3u)");
    //qDebug()<<strFile;
    QFile file(strFile);
    file.open(QIODevice::WriteOnly);
    file.close();
    // Exit function if no playlist is created
    if(strFile.isEmpty()&&strFile==""){
        return;
    }
    // Save selected playlist to user config, then set s_defaultPlaylist to selected playlist
    if(!strFile.isEmpty()&&strFile!=""){
    m_prefs.defaultPlaylist = strFile;
    s_defaultPlaylist = m_prefs.defaultPlaylist;
    saveSettings();
    }
    // Remove ratedabbr2 and run getPlaylist function, Set s_bool_PlaylistExist to true
    //removeAppData("ratedabbr2.txt");
    //removeAppData("ratedabbr2.txt");
    // Update excluded artists  (from step 13) by running function getExcludedArtists() which also recreates ratedabbr2, and check code1 stats
    if (Constants::kVerbose){std::cout << "on_actionNew_Playlist_triggered: Running getPlaylist." << std::endl;}
    getPlaylist(s_defaultPlaylist, s_musiclibrarydirname, s_musiclibshortened, s_topLevelFolderExists);
    if (Constants::kVerbose){std::cout << "on_actionNew_Playlist_triggered: Running getExcludedArtists." << std::endl;}
    //getExcludedArtists(s_playlistSize);
    s_bool_PlaylistSelected = true;
    // Get playlist size  (from step 11)
    s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
    if (s_playlistSize == 1) s_playlistSize = 0;
    if (Constants::kVerbose){std::cout << "on_actionNew_Playlist_triggered: Add/change playlist. s_playlistSize is: "<< s_playlistSize << std::endl;}
    // Recalculate historical count  (from step 12)
    s_histCount = int(s_SequentialTrackLimit - s_playlistSize);
    if (Constants::kVerbose){std::cout << "on_actionNew_Playlist_triggered: Add/change playlist. s_histCount is: "<< s_histCount << std::endl;}
    // Update excluded artists  (from step 13) by running function getExcludedArtists() which also recreates ratedabbr2, and check code1 stats
    getExcludedArtists(s_playlistSize);
    if (s_includeNewTracks){  // If user is including new tracks (from step 13), determine if a code 1 track should be added for this particular selection
        code1stats(&s_uniqueCode1ArtistCount,&s_code1PlaylistCount, &s_lowestCode1Pos, &s_artistLastCode1);// Retrieve rating code 1 stats
    }
    // Recheck album excludes if enabled (from step 14)
    if (s_includeAlbumVariety)
    {
        buildAlbumExclLibrary(s_minalbums, s_mintrackseach, s_mintracks);
        ui->albumsTab->setEnabled(true);
    }
    //Sets the playlist size limit to restrict how many tracks can be added to the playlist (from step 15)

    s_playlistSize = 0;
    s_playlistActualCntSelCode = 0;
    // Somewhat arbitrary guess on a reasonable starting max to use. Will be recalculated once tracks are added.
    s_MaxAvailableToAdd = int(static_cast<double>(playlistTrackLimitCodeQty) / static_cast<double>(selTrackLimitCodeRatingRatio))-50;
    playlistFull = false;

    // Finalize playlist loading
    ui->setgetplaylistLabel->setText("Selected: " + s_defaultPlaylist);
    ui->currentplsizeLabel->setText(tr("Current playlist size is ") + QString::number(s_playlistSize)+tr(" tracks, "));
    ui->playlistdaysLabel->setText(tr("and playlist length in listening days is ") + QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
    ui->addtrksspinBox->setValue(30);
    ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd-1,'g', 3));
    std::string MMdbDate = getMMdbDate(s_mmBackupDBDir);
    std::string LastTableDate = getLastTableDate();
    ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(MMdbDate)+
                                   tr(", Library date: ")+ QString::fromStdString(LastTableDate));
    ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(MMdbDate)+
                                   tr(", Library date: ")+ QString::fromStdString(LastTableDate));
    if (s_includeNewTracks){  // If user is including new tracks, determine if a code 1 track should be added for this particular selection
        s_uniqueCode1ArtistCount = 0;
        s_code1PlaylistCount = 0;
        s_lowestCode1Pos = Constants::kMaxLowestCode1Pos;
        code1stats(&s_uniqueCode1ArtistCount,&s_code1PlaylistCount, &s_lowestCode1Pos, &s_artistLastCode1);// Retrieve rating code 1 stats
        ui->newtracksqtyLabel->setText(tr("New tracks qty not in playlist: ") + QString::number(s_rCode1TotTrackQty - s_code1PlaylistCount));
    }
    ui->songsaddtextBrowser->setText("");
    ui->addsongsButton->setEnabled(true);
    ui->viewplaylistButton->setDisabled(true);
    ui->viewplaylistLabel->setText(tr("Current playlist is empty"));
    QFileInfo fi(s_defaultPlaylist);
    QString justname = fi.fileName();
    QMainWindow::setWindowTitle("ArchSimian - "+justname);
}

void ArchSimian::on_autosavecheckBox_stateChanged(int autosave)
{
    s_noAutoSave = autosave;
    m_prefs.s_noAutoSave = s_noAutoSave;
}

void ArchSimian::on_disablenotecheckBox_stateChanged(int disableNote)
{
    ui->disablenotecheckBox->checkState();
    m_prefs.s_disableNotificationAddTracks = disableNote;
    if (ui->disablenotecheckBox->checkState() == 2){
        m_prefs.s_disableNotificationAddTracks = true;
        QWidget::repaint();
    }
    if (ui->disablenotecheckBox->checkState() == 0){
        m_prefs.s_disableNotificationAddTracks = false;
        QWidget::repaint();
    }
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

void ArchSimian::on_viewplaylistButton_clicked()
{
    PlaylistContentDialog playlistcontentdialog;
    playlistcontentdialog.setModal(true);
    playlistcontentdialog.exec();
}

void ArchSimian::on_saveConfigButton_released()
{
    saveSettings();
    qApp->quit();
}

void ArchSimian::on_windowsDriveLtrEdit_textChanged(const QString &arg1)
{
    m_prefs.s_WindowsDriveLetter = arg1;
    s_winDriveLtr = m_prefs.s_WindowsDriveLetter;
    saveSettings();
}


void ArchSimian::on_playlistLimitSlider_valueChanged(int value)
{
    // Adjustment for track limit percentage. Set variable by dividing slider value by 100.

}
