#include <QSettings>
#include <QtWidgets>
#include <QPalette>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLayout>
#include <QProgressBar>
#include <QTimeLine>
#include <QProgressDialog>
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
#include "diagnostics.h"
#include "exporttoandroidaimp.h"
#include "archsimiantimer.h"
#include <id3/tag.h>
#include <filesystem>

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
static bool s_noAutoSave{false};
static bool s_disableNotificationAddTracks{false};
static int s_uniqueCode1ArtistCount{0};
static int s_code1PlaylistCount{0};
static int s_lowestCode1Pos{Constants::kMaxLowestCode1Pos};
static std::string s_artistLastCode1;
static std::string s_selectedCode1Path;
static std::string s_selectedTrackPath;
static std::string s_selectedRatingUpdate;
static std::string s_selectedTagUpdate;




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
/* Second group is declared to get recently played statistics for last six 10-day periods
 First range is between yesterday and eleven days ago, so first count is
 all values between (currSQLDate - 1) and (currSQLDate - 10.999)
 then (currSQLDate - 11) and (currSQLDate - 20.999) and so on to 60.999
 so anything greater than currSQLDate - 60.999 is counted, then subcounted to each 10 day period */
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
static QString s_androidpathname{""};
static QString s_syncthingpathname{""};
static bool s_mm4disabled{false};
static QString appDataPathstr = QDir::homePath() + "/.local/share/archsimian";
static QDir appDataPath = appDataPathstr;
static std::string cleanLibFile = appDataPathstr.toStdString()+"/cleanlib.dsv";
static int selectedTrackLimitCode{3};
static int selTrackLimitCodeTotTrackQty{0};
static double selTrackLimitCodeRatingRatio{0};
static int s_MaxAvailableToAdd{0};
static bool s_topLevelFolderExists{false};
static std::string playlistpath{""};
static int s_PlaylistLimit{0};
static int s_OpenPlaylistLimit{0};
static bool diagsran(0);
static bool s_audaciouslogenabled{0};
static bool s_initalpostsettingslaunch{0};


//static int timerfactor{0}; // take into account library size to determine timing interval


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
    s_mm4disabled = m_prefs.s_mm4disabled;
    s_androidpathname = m_prefs.s_androidpathname;
    s_syncthingpathname = m_prefs.s_syncthingpathname;
    s_audaciouslogenabled = m_prefs.s_audaciouslogenabled;
    s_initalpostsettingslaunch = m_prefs.s_initalpostsettingslaunch;

    // Set up the UI
    ui->setupUi(this);
    statusLabel = new QLabel(this);
    //statusProgressBar = new QProgressBar(this);
    ui->mainQTabWidget->setCurrentIndex(0);

    /* Step 1. Determine if user configuration exists:

     Use QSettings archsimian.conf file entries to check entries for mmBackupDBDir, mmPlaylistDir, and musicLibraryDir
     If any are blank, or the dir found does not exist, or file MM.DB does not exist at the location specified, then
     launch child window for user to set the locations prior to launch of main window. When child window
     is closed verify the locations and files selected exist.

     UI configuration: determine state of user config */
    if ((s_mmBackupDBDir != nullptr) && (s_musiclibrarydirname != nullptr) && (s_mmPlaylistDir != nullptr) &&(s_winDriveLtr != nullptr)){
        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 1. The locations s_mmBackupDBDir, s_musiclibrarydirname & mmPlaylistDir have all been set up." << std::endl;}
        s_bool_IsUserConfigSet = true;
    }
    else {
        std::cout << "Archsimian.cpp: Step 1. Unable to open archsimian.conf configuration file or, one or more locations has no data. s_bool_IsUserConfigSet result: "
                  << s_bool_IsUserConfigSet << std::endl;
    }

    //If configuration has already been set, populate the UI labels accordingly
    if (s_bool_IsUserConfigSet)
    {
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 1. Configuration was set. s_bool_IsUserConfigSet result: " << s_bool_IsUserConfigSet << std::endl;
        m_prefs.musicLibraryDir = s_musiclibrarydirname;
        ui->mmenabledradioButton_2->setEnabled(true);
        ui->mmdisabledradioButton->setEnabled(true);
        if (!m_prefs.s_mm4disabled){// MM4 is enabled, disable sync tab
            ui->mmenabledradioButton_2->setChecked(true);
            ui->mmdisabledradioButton->setChecked(false);
            ui->syncthingButton->setDisabled(true);
            ui->selectAndroidDeviceButton->setDisabled(true);
            ui->updateASDBButton->setDisabled(true);
            ui->enableAIMPOnlyradioButton->setDisabled(true);
            ui->enableAudaciousLogButton->setDisabled(true);
            ui->syncPlaylistButton->setDisabled(true);
            ui->updateratingsButton->setDisabled(true);
            ui->mainQTabWidget->setTabEnabled(5, false);
            ui->syncTab->setEnabled(false);
        }
        if (m_prefs.s_mm4disabled){// MM4 is disabled, enable (set disabled to false) sync tab
            ui->mmenabledradioButton_2->setChecked(false);
            ui->mmdisabledradioButton->setChecked(true);
            ui->syncthingButton->setDisabled(false);
            ui->selectAndroidDeviceButton->setDisabled(false);
            ui->updateASDBButton->setDisabled(false);
            ui->enableAIMPOnlyradioButton->setDisabled(false);
            ui->enableAudaciousLogButton->setDisabled(false);
            ui->syncPlaylistButton->setDisabled(false);
            ui->updateratingsButton->setDisabled(false);
            ui->mainQTabWidget->setTabEnabled(5, true);
            ui->syncTab->setEnabled(true);
        }
        if ((m_prefs.s_mm4disabled &&((s_androidpathname == "" ) || (s_syncthingpathname == "")))) { // If either of the 2 sync paths have not been established dim two action buttons and 2 radio buttons
                    ui->updateASDBButton->setDisabled(true);
                    ui->syncPlaylistButton->setDisabled(true);
                    ui->enableAudaciousLogButton->setDisabled(true);
                    ui->enableAIMPOnlyradioButton->setDisabled(true);
        }
        if (m_prefs.s_mm4disabled && !s_audaciouslogenabled){
            ui->enableAIMPOnlyradioButton->setChecked(true);
            ui->enableAudaciousLogButton->setChecked(false);
        }
        if (m_prefs.s_mm4disabled && s_audaciouslogenabled == true){
            ui->enableAIMPOnlyradioButton->setChecked(false);
            ui->enableAudaciousLogButton->setChecked(true);
        }
        if (s_includeNewTracks) {ui->InclNewcheckbox->setChecked(true);}
        if (s_includeAlbumVariety) {
            ui->albumscheckBox->setChecked(true);
            ui->mainQTabWidget->setTabEnabled(4, true);
        }
        if (!m_prefs.s_includeAlbumVariety){
            ui->mainQTabWidget->setTabEnabled(4, false);
            ui->albumsTab->setEnabled(false);
        }
        if (s_bool_CleanLibExist) {
            ui->mmdisabledradioButton->setEnabled(true);
            ui->mmenabledradioButton_2->setEnabled(true);
            ui->selectAndroidDeviceButton->setEnabled(true);
            ui->syncthingButton->setEnabled(true);
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
        ui->newmaxavailLabel->setText(tr("Maximum (est.) tracks available is: "));
        ui->statusBar->addPermanentWidget(statusLabel);
        ui->updateTagsprogressBar->setVisible(false);
        ui->updateASDBprogressBar->setVisible(false);
        ui->addsongsprogressBar->setVisible(false);
        ui->newmaxavailLabel->setText(tr("Maximum (est.) tracks available is: "));
        ui->androiddevicebuttonlabel->setText(tr("Select Android device (for syncing play history using AIMP - requires KDEconnect and AIMP with logging enabled.)"));
        if (s_androidpathname != ""){ui->androiddevicebuttonlabel->setText(s_androidpathname);}
        ui->syncthinglabel->setText(tr("Select the shared folder Syncthing will use to sync playlist and music files to Android device.)"));
        if (s_syncthingpathname != ""){ui->syncthinglabel->setText(s_syncthingpathname);}
        if (s_defaultPlaylist == ""){
            ui->setgetplaylistLabel->setText("No playlist selected");
            ui->addsongsButton->setEnabled(false);
            ui->viewplaylistButton->setDisabled(true);
            ui->viewplaylistLabel->setText(tr("No playlist selected"));
            QMainWindow::setWindowTitle("ArchSimian - No playlist selected");
        }
        ui->setmmdbButton->setEnabled(true);
        bool needUpdate = recentlyUpdated(s_mmBackupDBDir);
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 1. Running getMMdbDate(). MM.DB update need is: "<< needUpdate << std::endl;
        if (needUpdate == 0)
        {
            std::string MMdbDate = getMMdbDate(s_mmBackupDBDir);
            std::string LastTableDate = getLastTableDate();
            ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(MMdbDate)+
                                           tr(", Library date: ")+ QString::fromStdString(LastTableDate));
            if (s_mm4disabled){ui->updatestatusLabel->setText(tr("MM.DB date: Disabled, Library date: ")+ QString::fromStdString(LastTableDate));}
        }
    }
    // Step 2. Determine if the MediaMonkey (MM.DB) database exists with the doesFileExist (const std::string& name) function (sets s_bool_MMdbExist).
    if (s_bool_IsUserConfigSet) {
        std::string mmdbdir = s_mmBackupDBDir.toStdString();
        const std::string mmpath = mmdbdir + "/MM.DB";
        s_bool_MMdbExist = doesFileExist(mmpath);
        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 2. Does MM.DB file exist. s_bool_MMdbExist result: " << s_bool_MMdbExist << std::endl;}
    }
    //    a. If s_bool_IsUserConfigSet is false, set all three location variables to false and s_winDriveLtr to ""
    if (!s_bool_IsUserConfigSet) {
        s_bool_MMdbExist = false;
        s_bool_MMdbUpdated = false;
        s_bool_PlaylistExist = false;
        s_winDriveLtr = "";
    }
    //    b. If s_bool_IsUserConfigSet is true, s_mm4disabled is false (set to use MM.DB), but MM.DB was not found, set s_bool_IsUserConfigSet to false
    if ((s_bool_IsUserConfigSet) && (!s_bool_MMdbExist) && (!s_mm4disabled)) {
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 2. MM.DB was not found at the location you specified. Setting s_bool_IsUserConfigSet to false." << std::endl;
        s_bool_IsUserConfigSet = false;
    }
    // If configuration has not been set up completely, load UI setup instructions for user to set configuration locations
    if (!s_bool_IsUserConfigSet) {
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 2. Configuration has not been set. Adjusting UI settings" << std::endl;
        QMainWindow::setWindowTitle("ArchSimian - Initial Configuration Setup");
        ui->setlibrarylabel->setText(tr(""));
        ui->setlibraryButton->setEnabled(true);
        ui->setmmpllabel->setText(tr(""));
        ui->setmmplButton->setEnabled(false);
        ui->setmmdblabel->setText(tr(""));
        ui->setmmdbButton->setEnabled(false);
        ui->saveConfigButton->setEnabled(false);
        ui->menuBar->setDisabled(true);
        ui->autosavecheckBox->setChecked(true);
        ui->windowsDriveLtrEdit->setText(tr(""));
        ui->windowsDriveLtrEdit->setEnabled(false);
        ui->mmdisabledradioButton->setEnabled(false);
        ui->mmenabledradioButton_2->setEnabled(false);
        ui->selectAndroidDeviceButton->setEnabled(false);
        ui->syncthingButton->setEnabled(false);

        ui->setgetplaylistLabel->setText(tr("Select playlist for adding tracks"));
        ui->mainQTabWidget->setCurrentIndex(1);
        ui->instructionlabel->setEnabled(true);
        ui->setlibrarylabel->setText(tr("Set the home directory (top level) of the music library and store in user settings."));
        ui->setmmpllabel->setText(tr("Select the shared Windows directory where you store your m3u playlists."));
        ui->setmmdblabel->setText(tr("Select the shared Windows directory where you stored the MediaMonkey database (MM.DB) backup file."));
        ui->instructionlabel->setText(tr("ArchSimian Setup: \n(1) Identify the location where your music library is stored  \n(2) Set the "
                                         "location where you did exports of your M3U playlists from MM4 \n(3) Set the location where you did backup "
                                         "of the MM.DB file \n(4) Check whether to enable new tracks \n(5) Check whether to enable album-level "
                                         "variety. \n(6) Enter the Windows drive letter of the music library. \n"
                                         "(7) Close (which saves the locations and settings) and restart the program."));
       ui->songsaddtextBrowser->setText(tr("After ArchSimian setup completed, and before creating/revising playlist, first review and "
                                                     "adjust playlist rating allocation using the Frequency tab. "));
    }

    /* Step 3. Determine if Archsimian songs table exists. If it does not, run diagnostics. If user configuration exists
     and MM4 data exists (s_bool_IsUserConfigSet and s_bool_MMdbExist are true),
     determine if cleanlib.dsv songs table (cleanLibFile) exists in AS, function doesFileExist (const std::string& name)  (dir paths corrected,
     imported from MM.DB) (sets s_bool_CleanLibExist) */
    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (!s_mm4disabled)) {
        bool tmpbool{true};
        tmpbool = doesFileExist(cleanLibFile);
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 3. cleanLib file: " << cleanLibFile <<
                                              "  exists result: " << tmpbool << std::endl;
        if (!tmpbool){
            // If cleanlib does not exist (new installation), run diagnostics before generating library
            if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 3. Generating a diagnostic check on new installation." << std::endl;
            std::string logd = appDataPathstr.toStdString()+"/diagnosticslog.txt";
            removeAppData(logd);
            generateDiagsLog();
            diagsran = true;
        }
        if (tmpbool){ // check that file is not empty
            //Check whether the songs table currently has any data in it
            std::streampos cleanLibFilesize;
            char * memblock;
            std:: ifstream file (cleanLibFile, std::ios::in|std::ios::binary|std::ios::ate);
            if (file.is_open())
            {
                cleanLibFilesize = file.tellg();
                memblock = new char [static_cast<unsigned long>(cleanLibFilesize)];
                file.seekg (0, std::ios::beg);
                file.read (memblock, cleanLibFilesize);
                file.close();
                delete[] memblock;
            }
            if (cleanLibFilesize != 0) {s_bool_CleanLibExist = true;}
            if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 3. cleanLib file not empty result: " << cleanLibFilesize << std::endl;
        }
    }
    if ((Constants::kVerbose)&&(s_bool_IsUserConfigSet)&& (!s_mm4disabled)) std::cout << "Archsimian.cpp: Step 3. Does CleanLibFile exist. s_bool_CleanLibExist result: "
                                                                   << s_bool_CleanLibExist << std::endl;

    /* Step 3a. If MM4 update is disabled (s_mm4disabled is true), determine if Archsimian songs table exists. If it does not, run diagnostics. If user
     configuration exists (s_bool_IsUserConfigSet and s_mm4disabled are true),
     determine if cleanlib.dsv songs table (cleanLibFile) exists in AS, function doesFileExist (const std::string& name)  (dir paths corrected,
     imported from MM.DB) (sets s_bool_CleanLibExist) */
    if ((s_bool_IsUserConfigSet) && (s_mm4disabled)) {
        bool tmpbool{true};
        tmpbool = doesFileExist(cleanLibFile);
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 3a. cleanLib file: " << cleanLibFile <<
                                              "  exists result: " << tmpbool << std::endl;
        //if (!tmpbool){
            // If cleanlib does not exist (was deleted), restore cleanLibFile from backup copy
          //  if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 3a. Regenerating cleanLibFile. File was deleted for unknown reason." << std::endl;
        // }
        if (tmpbool){ // check that file is not empty
            //Check whether the songs table currently has any data in it
            std::streampos cleanLibFilesize;
            char * memblock;
            std:: ifstream file (cleanLibFile, std::ios::in|std::ios::binary|std::ios::ate);
            if (file.is_open())
            {
                cleanLibFilesize = file.tellg();
                memblock = new char [static_cast<unsigned long>(cleanLibFilesize)];
                file.seekg (0, std::ios::beg);
                file.read (memblock, cleanLibFilesize);
                file.close();
                delete[] memblock;
            }
            if (cleanLibFilesize != 0) {s_bool_CleanLibExist = true;}
            if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 3a. cleanLib file not empty result: " << cleanLibFilesize << std::endl;
        }
    }


    // Add NEW code here for restoring backup of cleanLibFile.




    /* 4. Determine if MM.DB was recently updated unless MM4 update has been disabled. s_bool_MMdbUpdated is set by comparing MM.DB file date
     to CleanLib (songs table) file date. If the MM.DB file date is newer (greater) than the CleanLib file date
     will need to run diagnostics and update. */

    if ((s_bool_IsUserConfigSet) && (!s_mm4disabled)) {
        s_bool_MMdbUpdated = recentlyUpdated(s_mmBackupDBDir);
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 4. Is the MM.DB file date newer (greater) than the CleanLibFile date."
                                              " s_bool_MMdbUpdated result: "<< s_bool_MMdbUpdated << std::endl;
        // set UI labels if MM.DB was recently updated
        if (s_bool_MMdbUpdated) // bool s_bool_MMdbUpdated: 1 means refresh DB, 0 means skip
            // If newer is true, removeAppData ratedabbr.txt, ratedabbr2.txt, artistsadj.txt, playlistposlist.txt, artistexcludes.txt, and cleanedplaylist.txt files
        {
            removeAppData("ratedabbr.txt");
            s_bool_RatedAbbrExist = false;
            removeAppData("ratedabbr2.txt");
            removeAppData("artistsadj.txt");
            s_bool_artistsadjExist = false;
            //removeAppData("playlistposlist.txt");

            //removeAppData("artistexcludes.txt");

            s_bool_ExcludedArtistsProcessed = false;
            removeAppData("cleanedplaylist.txt");
            s_bool_PlaylistExist = false;
            // If cleanlib does not exist (new installation), run diagnostics before generating library if not already run (bool diagsran)
            if (diagsran == false){
                if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 4. Generating a diagnostic check after database update." << std::endl;
                std::string logd = appDataPathstr.toStdString()+"/diagnosticslog.txt";
                removeAppData(logd);
                generateDiagsLog();
                ui->updatestatusLabel->setText(tr("MM.DB was recently backed up. Library has been rebuilt and diagnostics log generated."));
            }
            ui->updatestatusLabel->setText(tr("MM.DB was recently backed up. Library has been rebuilt and diagnostics log generated."));
        }
    }

    /* Step 5. Unless MM4 update has been disabled, if user configuration and MM4 data exist, but the songs table does not, import songs table (from MM.DB to libtable.dsv to cleanlib.dsv)
     into Archsimian: If user configuration and MM4 data exist, but the songs table does not (bool_IsUserConfigSet, s_bool_MMdbExist are true,
     s_bool_CleanLibExist is false), import songs table into AS, by running writeSQLFile() function, which creates the temporary basic table file
     libtable.dsv; then run the getLibrary() function, which creates the refined table file cleanlib.dsv The getLibrary() function completes the
     following refinements: (a) corrects the directory paths to Linux, (b) adds random lastplayed dates for rated or "new-need-to-be-rated"
     tracks that have no play history, (c) creates rating codes for any blank values found in GroupDesc col, using POPM values,
     and (d) creates Artist codes (using col 1) and places the code in Custom2 if Custom2 is blank. Then set s_bool_CleanLibExist to true, rechecking,
     run doesFileExist (const std::string& name) function. After verifying  cleanlib.dsv exists, remove temporary basic table file
     libtable.dsv Evaluates s_bool_CleanLibExist for existence of cleanlib.dsv (cleanLibFile) */

    if ((Constants::kVerbose)&&(!s_bool_MMdbUpdated)&&(s_bool_CleanLibExist)&&(s_bool_IsUserConfigSet)&&(!s_mm4disabled)){
        std::cout << "Archsimian.cpp: Step 5. CleanLib file exists and MM.DB was not recently updated."<< std::endl;}

    if (((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (!s_bool_CleanLibExist)&&(!s_mm4disabled)) || (s_bool_MMdbUpdated)) {
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 5. User configuration and MM.DB exists, but the songs table does not, or MM.DB was "
                                              "recently updated. Importing songs table (create CleanLib) from MM.DB." <<std::endl;
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
        else if (c_pid > 0){  /* Parent process starts here. Write from libtable.dsv and gather stats
            First, reopen libtable.dsv, clean track paths, and output to cleanlib.dsv
            Check to ensure the file has finshed being written */
            if( (c_pid = wait(&status)) < 0){
                perror("wait");
                _exit(1);
            }
            if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 5: Generating Cleanlib file. "<< std::endl;}
            getLibrary(s_musiclibrarydirname,&s_musiclibshortened, &s_windowstopfolder); // get songs table from MM.DB
            removeSQLFile();
            m_prefs.s_windowstopfolder = s_windowstopfolder;
            m_prefs.s_musiclibshortened = s_musiclibshortened;
            if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 5: s_musiclibshortened: "
                                               << s_musiclibshortened.toStdString()<< " and s_windowstopfolder: "<<s_windowstopfolder.toStdString() << std::endl;
            s_bool_CleanLibExist = doesFileExist (cleanLibFile);
        }
    }
    //Step 5a. Set bool for 'does top level windows folder exist' from user config
    if (s_windowstopfolder.toStdString()== ""){
        s_topLevelFolderExists = false;
        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 5: Top level windows folder does not exist result: "<<s_topLevelFolderExists<< std::endl;}
    }
    else{
        s_topLevelFolderExists = true;
        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 5: Top level windows folder is "<<s_windowstopfolder.toStdString()<< std::endl;}
    }

    /* Step 6. If user configuration exists, MM.DB exists and songs table exists, process/update statistics: If user configuration exists, MM4 data exists,
     songs table exists (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist are all true), run function to process/update statistics getDBStats()

    6a. Treat MM.DB as true and needs update if MM4 is disabled, for rest of setup */
    if (s_mm4disabled){
        s_bool_MMdbExist = true;
        s_bool_MMdbUpdated = false;
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 6a: Treating MM.DB as needing update for rest of setup."<< std::endl;
    }
    if ((Constants::kVerbose)&&(s_bool_IsUserConfigSet)&&(s_bool_MMdbExist)&&(s_bool_CleanLibExist)) {std::cout
            << "Archsimian.cpp: Step 6. User configuration exists, MM.DB exists and songs table exists. Processing database statistics." << std::endl;}
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
        if (Constants::kVerbose) { //Print verbose results to console
            std::cout << "Archsimian.cpp: Step 6. selTrackLimitCodeTotTrackQty is: "<< selTrackLimitCodeTotTrackQty << std::endl;
            std::cout << "Archsimian.cpp: Step 6. selectedTrackLimitCode is: "<< selectedTrackLimitCode << std::endl;
            std::cout << "Archsimian.cpp: Step 6. selTrackLimitCodeRatingRatio is: "<< selTrackLimitCodeRatingRatio << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks Rating 0 - s_rCode0TotTrackQty : " << s_rCode0TotTrackQty << ". Total Time (hrs) - s_rCode0TotTime : " <<  s_rCode0TotTime << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks Rating 1 - s_rCode1TotTrackQty : " << s_rCode1TotTrackQty << ". Total Time (hrs) - s_rCode1TotTime : " <<  s_rCode1TotTime << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks Rating 3 - s_rCode3TotTrackQty : " << s_rCode3TotTrackQty << ". Total Time (hrs) - s_rCode3TotTime : " <<  s_rCode3TotTime << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks Rating 4 - s_rCode4TotTrackQty : " << s_rCode4TotTrackQty << ". Total Time (hrs) - s_rCode4TotTime : " <<  s_rCode4TotTime << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks Rating 5 - s_rCode5TotTrackQty : " << s_rCode5TotTrackQty << ". Total Time (hrs) - s_rCode5TotTime : " <<  s_rCode5TotTime << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks Rating 6 - s_rCode6TotTrackQty : " << s_rCode6TotTrackQty << ". Total Time (hrs) - s_rCode6TotTime : " <<  s_rCode6TotTime << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks Rating 7 - s_rCode7TotTrackQty : " << s_rCode7TotTrackQty << ". Total Time (hrs) - s_rCode7TotTime : " <<  s_rCode7TotTime << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks Rating 8 - s_rCode8TotTrackQty : " << s_rCode8TotTrackQty << ". Total Time (hrs) - s_rCode8TotTime : " <<  s_rCode8TotTime << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks in the library is - s_totalLibQty : " << s_totalLibQty << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total rated tracks in the library is - s_totalRatedQty : " << s_totalRatedQty << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total rated time in the library is - s_TotalRatedTime : " <<s_totalRatedTime << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total time listened for first 10-day period is (hrs) - s_SQL10TotTimeListened : " << s_SQL10TotTimeListened << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks played in the first period is - s_SQL10DayTracksTot : " << s_SQL10DayTracksTot << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total time listened for second 10-day period is (hrs) - s_SQL20TotTimeListened : " << s_SQL20TotTimeListened << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks played in the second period is - s_SQL20DayTracksTot : " << s_SQL20DayTracksTot << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total time listened for third 10-day period is (hrs): " << s_SQL30TotTimeListened << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks played in the third period is - s_SQL30TotTimeListened : " << s_SQL30DayTracksTot << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total time listened for fourth 10-day period is (hrs) - s_SQL40TotTimeListened : " << s_SQL40TotTimeListened << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks played in the fourth period is - s_SQL40DayTracksTot : " << s_SQL40DayTracksTot << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total time listened for fifth 10-day period is (hrs) - s_SQL50TotTimeListened : " << s_SQL50TotTimeListened << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks played in the fifth period is - s_SQL50DayTracksTot : " << s_SQL50DayTracksTot << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total time listened for sixth 10-day period is (hrs) - s_SQL60TotTimeListened : " << s_SQL60TotTimeListened << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total tracks played in the sixth period is - s_SQL60DayTracksTot : " << s_SQL60DayTracksTot << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total time listened for the last 60 days is (hrs) - s_totHrsLast60Days : " << s_totHrsLast60Days << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Calculated daily listening rate is (hrs) - s_listeningRate : "<< s_listeningRate << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Years between repeats code 3 - s_yrsTillRepeatCode3 : "<< s_yrsTillRepeatCode3 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Years between repeats code 4 - s_yrsTillRepeatCode4 : "<< s_yrsTillRepeatCode4 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Years between repeats code 5 - s_yrsTillRepeatCode5 : "<< s_yrsTillRepeatCode5 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Years between repeats code 6 - s_yrsTillRepeatCode6 : "<< s_yrsTillRepeatCode6 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Years between repeats code 7 - s_yrsTillRepeatCode7 : "<< s_yrsTillRepeatCode7 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Years between repeats code 8 - s_yrsTillRepeatCode8 : "<< s_yrsTillRepeatCode8 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Adjusted hours code 3 - s_adjHoursCode3 : "<< s_adjHoursCode3 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Adjusted hours code 4 - s_adjHoursCode4 : "<< s_adjHoursCode4 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Adjusted hours code 5 - s_adjHoursCode5 : "<< s_adjHoursCode5 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Adjusted hours code 6 - s_adjHoursCode6 : "<< s_adjHoursCode6 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Adjusted hours code 7 - s_adjHoursCode7 : "<< s_adjHoursCode7 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Adjusted hours code 8 - s_adjHoursCode8 : "<< s_adjHoursCode8 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total Adjusted Hours - s_totAdjHours : "<< s_totAdjHours << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Total Adjusted Quantity - s_totalAdjRatedQty : "<< s_totalAdjRatedQty << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Percentage of track time for scheduling rating code 3 - s_ratingRatio3 * 100 : "<< s_ratingRatio3 * Constants::kConvertDecimalToPercentDisplay << "%" << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Percentage of track time for scheduling rating code 4 - s_ratingRatio4 * 100 : "<< s_ratingRatio4  * Constants::kConvertDecimalToPercentDisplay << "%" << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Percentage of track time for scheduling rating code 5 - s_ratingRatio5 * 100 : "<< s_ratingRatio5  * Constants::kConvertDecimalToPercentDisplay << "%" << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Percentage of track time for scheduling rating code 6 - s_ratingRatio6 * 100 : "<< s_ratingRatio6 * Constants::kConvertDecimalToPercentDisplay <<  "%" << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Percentage of track time for scheduling rating code 7 - s_ratingRatio7 * 100 : "<< s_ratingRatio7 * Constants::kConvertDecimalToPercentDisplay <<  "%" << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Percentage of track time for scheduling rating code 8 - s_ratingRatio8 * 100 : "<< s_ratingRatio8 * Constants::kConvertDecimalToPercentDisplay <<  "%" << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Number of days until track repeat under rating code 3 - s_DaysBeforeRepeatCode3 : "<< s_DaysBeforeRepeatCode3 << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Average length of rated songs in fractional minutes - s_AvgMinsPerSong : "<< s_AvgMinsPerSong << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Calculated daily listening rate in mins - s_avgListeningRateInMins : "<< s_avgListeningRateInMins << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Calculated tracks per day - s_avgListeningRateInMins / s_AvgMinsPerSong : "<< s_avgListeningRateInMins / s_AvgMinsPerSong << std::endl;
            std::cout << "Archsimian.cpp: Step 6. Sequential Track Limit - s_SequentialTrackLimit : "<< s_SequentialTrackLimit << std::endl;
        }
        s_bool_dbStatsCalculated = true; // Set bool to true for s_bool_dbStatsCalculated
        ui->daystracksLabel->setText(QString::number((m_prefs.tracksToAdd * s_AvgMinsPerSong)/s_avgListeningRateInMins,'g', 3));
    }
    else {
        s_bool_dbStatsCalculated = false;
        if (s_bool_IsUserConfigSet){
            std::cout << "Archsimian.cpp: Step 6. ERROR: Something went wrong processing the function getDBStats." << std::endl;
        }
    }

    /* Step 7a. If user configuration exists, MM.DB exists, songs table exists, statistics are processed, and
     MM.DB was not recently updated, check for state of s_bool_artistsadjExist (artistsadj.txt).
     If file is missing or empty, create file with artist statistics */

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
            else std::cout << "Archsimian.cpp: Step 7a. ERROR: There was a problem opening artistsadj.txt" << std::endl;
            if (artsistAdjsize != 0) {
                s_bool_artistsadjExist = true;// file artistsadj.txt exists and is greater in size than zero, set to true
                // If MM.DB not recently updated and artistsadj.txt does not need to be updated, check if ratedabbr.txt exists
                // If it does set s_bool_RatedAbbrExist to true.
                if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 7a. MM.DB not recently updated and artistsadj.txt does not need to be updated. "
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
                    else std::cout << "Archsimian.cpp: Step 7a. There was a problem opening ratedabbr.txt" << std::endl;
                    if (ratedabbrsize != 0) {
                        s_bool_RatedAbbrExist = true;
                        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 7a. Set s_bool_RatedAbbrExist = true." << std::endl;}
                    }
                }
            }
            if (artsistAdjsize == 0) {s_bool_artistsadjExist = false;}// file exists but size is zero, set to false
        }
        if (!tmpbool){s_bool_artistsadjExist = false;} // file does not exist, set bool to false
        if (s_mm4disabled){
            s_bool_artistsadjExist = false; // If MM4 is disabled, set bool to false
            if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 7a. MM4 is disabled. s_bool_artistsadjExist set to false." << std::endl;
        }
        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 7a. MM.DB not recently updated. Verifying artistsadj.txt exists and is not zero. "
                                              "s_bool_artistsadjExist result: "<< s_bool_artistsadjExist << std::endl;}
        if (!s_bool_artistsadjExist){
            getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                                   &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                                   &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                                   &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);
            s_bool_artistsadjExist = doesFileExist (appDataPathstr.toStdString()+"/artistsadj.txt");
            s_bool_RatedAbbrExist = false;
            if (!s_bool_artistsadjExist)  {std::cout << "Archsimian.cpp: Step 7a Something went wrong at the function getArtistAdjustedCount. artistsadj.txt not created." << std::endl;}
        }
    }

    /* Step 7b. If user configuration exists, MM.DB exists, songs table exists, statistics are processed, and
     MM.DB was recently updated, create file with artist statistics */

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist) &&
            (s_bool_dbStatsCalculated) && (s_bool_MMdbUpdated)) {
        if (Constants::kVerbose) {std::cout << "Archsimian.cpp: Step 7b. MM.DB was recently updated. Processing artist statistics." << std::endl;}
        getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                               &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                               &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                               &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);
        s_bool_RatedAbbrExist = false;
        s_bool_artistsadjExist = doesFileExist (appDataPathstr.toStdString()+"/artistsadj.txt");
        if (!s_bool_artistsadjExist)  {std::cout << "Archsimian.cpp: Step 7b. Something went wrong at the function getArtistAdjustedCount. artistsadj.txt not created." << std::endl;}
    }

    /* 8.  If user configuration exists, MM.DB exists, songs table exists, database statistics exist, artist statistics are processed, create
     a modified database with only rated tracks and which include artist intervals calculated for each: If user configuration exists,
     MM4 data exists, songs table exists, database statistics exist, and file artistsadj.txt is created (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist,
     s_bool_dbStatsCalculated, bool10 are all true), run function getSubset() to create a modified database file with rated tracks
     only and artist intervals for each track, rechecking, run doesFileExist (const std::string& name) (ratedabbr.txt) function (s_bool_RatedAbbrExist) */

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist)  && (s_bool_dbStatsCalculated)
            && (s_bool_artistsadjExist) && ((!s_bool_RatedAbbrExist) || (s_mm4disabled))) {
        getSubset();
        s_bool_RatedAbbrExist = doesFileExist (appDataPathstr.toStdString()+"/ratedabbr.txt");
        if (!s_bool_RatedAbbrExist)  {std::cout << "Archsimian.cpp: Step 8. Something went wrong at the function getSubset(). ratedabbr.txt not created." << std::endl;}
        if ((s_bool_RatedAbbrExist) && (Constants::kVerbose)){std::cout << "Archsimian.cpp: Step 8. ratedabbr.txt was created." << std::endl;}
    }
    else {
        if ((Constants::kVerbose) && (s_bool_RatedAbbrExist) && (s_bool_IsUserConfigSet)){std::cout
                    << "Archsimian.cpp: Step 8. MM.DB and artist.adj not recently updated. ratedabbr.txt not updated." << std::endl;}
    }

    /* 9. Checks whether there is a configuration entry for a default playlist. Sets bool for s_bool_PlaylistSelected.
     Also sets s_bool_PlaylistExist to false to force reloading of the default playlist (if it exists) each time
     the program starts.*/

    s_bool_PlaylistExist = false;
    s_bool_PlaylistSelected = true;
    if (s_defaultPlaylist == ""){
        s_bool_PlaylistSelected = false;
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 9. A default playlist does not exist. Playlist not selected." << std::endl;}
        }
        else {
            if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 9. Default playlist found and selected." << std::endl;}
        }

    /* 10. If a playlist was identified in the user config (step 9), generate the playlist file: If user configuration
     exists, MM4 data exists, songs table exists (bool_IsUserConfigSet, s_bool_MMdbExist, s_bool_CleanLibExist are all true), and playlist from user config exists
     (s_bool_PlaylistSelected is true), run function to generate cleaned playlist file getPlaylist()
     then set s_bool_PlaylistExist to true, rechecking, run doesFileExist (const std::string& name) function. Evaluates s_bool_PlaylistExist and sets to true
     (after running getPlaylist) if initially false */

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist) && (s_bool_PlaylistSelected) && (s_topLevelFolderExists)){
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 10. Regenerating 'cleanedplaylist' file for editing using default playlist identified in user config." << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 10. Running getPlaylist for this default playlist: "
                                              << s_defaultPlaylist.toStdString()<< std::endl;}
        getPlaylist(s_defaultPlaylist, s_musiclibrarydirname, s_musiclibshortened, s_topLevelFolderExists);
        s_bool_PlaylistExist = doesFileExist (appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        QFileInfo fi(s_defaultPlaylist);
        QString justname = fi.fileName();
        QMainWindow::setWindowTitle("ArchSimian - "+justname);
        if (!s_bool_PlaylistExist) {std::cout << "Archsimian.cpp: Step 10. Something went wrong at the function getPlaylist." << std::endl;}
    }

    // 10a. If a playlist was not identified in the user config, adjust the UI accordingly

    if ((s_bool_IsUserConfigSet) && (s_bool_MMdbExist) && (s_bool_CleanLibExist) && (!s_bool_PlaylistSelected) && (!s_bool_PlaylistExist)){
        ui->setgetplaylistLabel->setText("No playlist selected");
        ui->addsongsButton->setEnabled(false);
        ui->addsongsLabel->setText(tr(""));
        ui->viewplaylistButton->setDisabled(true);
        ui->viewplaylistLabel->setText(tr("No playlist selected"));
        QMainWindow::setWindowTitle("ArchSimian - No playlist selected");
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 10. No default playlist selected. "<<std::endl;
        }
    }

    /* 11. If playlist exists, calculate the playlist size: If cleaned playlist exists (s_bool_PlaylistExist is true), obtain playlist size
     using function cstyleStringCount(),  s_playlistSize = cstyleStringCount(cleanedPlaylist); And, get the Windows drive letter.*/
    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet)) {
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        if (s_playlistSize < 2) {
            s_playlistSize = 0;
            ui->viewplaylistButton->setDisabled(true);
            ui->viewplaylistLabel->setText(tr("Current playlist is empty"));
            QMainWindow::setWindowTitle("ArchSimian - No playlist selected");
            if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 11. No playlist selected. Playlist size is: "<< s_playlistSize << std::endl;}
        }
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 11. Playlist size is: "<< s_playlistSize << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 11. Windows drive letter loaded from user configuration "
                                              "as: "<< s_winDriveLtr.toStdString() << std::endl;}
    }

    /* 12. If playlist exists, obtain the historical count (in addition to the playlist count) up to the sequential track limit:
     If cleaned playlist exists (s_bool_PlaylistExist is true), obtain the historical count (in addition to the playlist count) up to the
     sequential track limit. A variable is needed (which later will be used to obtain additional play history outside of
     playlist, as part of a later function to make a new track selection), using the variable s_histCount. The value is
     calculated [ can be modified to use the function to an added function like void getHistCount(&s_SequentialTrackLimit,&s_playlistSize),
     or just: s_histCount = long(s_SequentialTrackLimit) – long(s_playlistSize); this uses both playlist size from 10
     and SequentialTrackLimit obtained with data from function getDBStats() */

    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet)) {
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        s_histCount = int(s_SequentialTrackLimit - s_playlistSize);
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 12. s_histCount is: "<< s_histCount << std::endl;}
    }
    else {
        s_histCount = int(s_SequentialTrackLimit);
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 12. No playlist selected. s_histCount is: "<< s_histCount << std::endl;}
    }

    /* 13. After artist statistics are processed, create/update excluded artists
     list. getExcludedArtists creates temporary database (ratedabbr2.txt) with playlist position numbers for use in subsequent functions. If playlist does not
     exist, it copies ratedabbr.txt to ratedabbr2.txt. getExcludedArtistsRedux continues excluded artist by accounting for excluded artists not in a playlist.

    create/update excluded artists list using vectors
     read in from the following files: cleanlib.dsv, artistsadj.txt, and cleanedplaylist.txt. Writes artistexcludes.txt. */

    if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 13. Processing artist stats and excluded artists list. Creating temporary database (ratedabbr2.txt)"<< std::endl;}
    if (Constants::kVerbose){std::cout << "                         with playlist position numbers for use in subsequent functions, ratingCodeSelected and selectTrack."<< std::endl;}

    if (s_bool_IsUserConfigSet && s_bool_CleanLibExist) {
        getExcludedArtists(s_playlistSize);
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 13. Completed getExcludedArtists."<< std::endl;}
        getExcludedArtistsRedux(s_playlistSize, int(s_histCount));
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 13. Completed getExcludedArtistsRedux."<< std::endl;}
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
        ui->addtrksspinBox->setMaximum(100);
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
        if (m_prefs.s_mm4disabled){
            ui->mmenabledradioButton_2->setChecked(false);
            ui->mmdisabledradioButton->setChecked(true);
            ui->syncthingButton->setDisabled(false);
            ui->selectAndroidDeviceButton->setDisabled(false);
            ui->updateASDBButton->setDisabled(false);
            ui->enableAIMPOnlyradioButton->setDisabled(false);
            ui->enableAudaciousLogButton->setDisabled(false);
            ui->syncPlaylistButton->setDisabled(false);
            ui->updateratingsButton->setDisabled(false);
            ui->mainQTabWidget->setTabEnabled(5, true);
            ui->syncTab->setEnabled(true);
        }
        if (!m_prefs.s_mm4disabled){
            ui->mmenabledradioButton_2->setChecked(true);
            ui->mmdisabledradioButton->setChecked(false);
            ui->syncthingButton->setDisabled(true);
            ui->selectAndroidDeviceButton->setDisabled(true);
            ui->updateASDBButton->setDisabled(true);
            ui->enableAIMPOnlyradioButton->setDisabled(true);
            ui->enableAudaciousLogButton->setDisabled(true);
            ui->syncPlaylistButton->setDisabled(true);
            ui->updateratingsButton->setDisabled(true);
            ui->mainQTabWidget->setTabEnabled(5, false);
            ui->syncTab->setEnabled(false);
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

        //ui->mainQTabWidget->setTabEnabled(5, true);
    }
    ui->minalbumsspinBox->setValue(m_prefs.s_minalbums);
    ui->mintracksspinBox->setValue(m_prefs.s_mintracks);
    ui->mintrackseachspinBox->setValue(m_prefs.s_mintrackseach);
    ui->totratedtimefreqLabel->setText("Total time (in hours) is:               " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
    ui->totadjhoursfreqLabel->setText("Total adjusted time (in hours) is: " + QString::number(((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
                                                                                              ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
                                                                                              ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
                                                                                              ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
                                                                                              ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
                                                                                              ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime)));
    // Calculated daily listening rate in hrs * 365 = listening hours per year
    // s_listeningRate * 365
    //     ui->totallisteninghrsyrlabel->setText("Total projected listening time (in hours) per year is: " + QString::fromStdString(std::to_string(int(s_listeningRate * 365))));
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

    // If an initial lauch after user setup completed, launch with only the Frequency tab enabled
    if (s_initalpostsettingslaunch == true){
        ui->mainQTabWidget->setTabEnabled(0, false);
        ui->mainQTabWidget->setTabEnabled(1, false);
        ui->mainQTabWidget->setTabEnabled(2, false);
        ui->mainQTabWidget->setTabEnabled(3, true);
        ui->mainQTabWidget->setTabEnabled(4, false);
        ui->mainQTabWidget->setTabEnabled(5, false);
        ui->frequencyTab->setEnabled(true);
    }

    // If the above ui config was for initial config with only the Frequency tab set the variable to disable next launch
    if (s_initalpostsettingslaunch == true){
        s_initalpostsettingslaunch = false;
        m_prefs.s_initalpostsettingslaunch = s_initalpostsettingslaunch;
    }


    /* 14. If user selects bool for s_includeAlbumVariety, run function buildAlbumExclLibrary(const int &s_minalbums,
      const int &s_mintrackseach, const int &s_mintracks). When basiclibrary functions are processed
      (at startup), this function gets artists meeting the screening criteria if the user selected album-level variety.
      This generates the file artistalbmexcls.txt (see albumexcludes project) */

    if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 14. If user selects bool for s_includeAlbumVariety, run function buildAlbumExclLibrary."<< s_includeAlbumVariety << std::endl;}

    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet) && (s_includeAlbumVariety))
    {
        buildAlbumExclLibrary(s_minalbums, s_mintrackseach, s_mintracks);
        ui->albumsTab->setEnabled(true);
    }
    // 15. Sets the playlist size limit to restrict how many total tracks can be added to any playlist

    if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. Run setPlaylistLimitCount to set the initial count for playlistLimitCount." << std::endl;}
    // Set variables for playlist limit applicable for all playlists created
    if (s_bool_IsUserConfigSet)
    {
        double tracksPerDay = (s_avgListeningRateInMins) / (s_AvgMinsPerSong);
        double interim1 = (tracksPerDay * selTrackLimitCodeRatingRatio * s_DaysBeforeRepeatCode3);
        int firstlimittest = int((selTrackLimitCodeTotTrackQty - interim1)/selTrackLimitCodeRatingRatio);
        int secondlimittest = int(tracksPerDay * s_DaysBeforeRepeatCode3 * 0.95);
        s_PlaylistLimit = std::min(firstlimittest,secondlimittest) - 20; //
        s_MaxAvailableToAdd = s_PlaylistLimit; // In case there is no default playlist, set s_MaxAvailableToAdd to calculated limit

        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. selTrackLimitCodeTotTrackQty: "<< selTrackLimitCodeTotTrackQty << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. tracksPerDay:(s_avgListeningRateInMins) / (s_AvgMinsPerSong)-> "<< tracksPerDay << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. selTrackLimitCodeRatingRatio: "<< selTrackLimitCodeRatingRatio << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. s_DaysBeforeRepeatCode3: "<< s_DaysBeforeRepeatCode3 << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. interim1: "<< interim1 << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. firstlimittest: "<< firstlimittest << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. secondlimittest: "<< secondlimittest << std::endl;}
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: Step 15. s_PlaylistLimit = smaller of firstlimittest and secondlimittest: "<< s_PlaylistLimit << std::endl;}
    }
    if ((s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet))
    {
        /* If a default playlist is found, set s_OpenPlaylistLimit to the s_PlaylistLimit
         Then, set s_MaxAvailableToAdd, using the s_OpenPlaylistLimit less the number of tracks already in the playlist */
        s_OpenPlaylistLimit = s_PlaylistLimit;
        s_MaxAvailableToAdd = s_OpenPlaylistLimit - s_playlistSize;
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 15. Default playlist found. Setting "
                                              "s_MaxAvailableToAdd = s_OpenPlaylistLimit - s_playlistSize: "<< s_MaxAvailableToAdd << std::endl;
        // Set labels, UI settings for when playlist is full
        if (s_MaxAvailableToAdd < 1){
            s_MaxAvailableToAdd = 0;
            ui->addsongsButton->setEnabled(false);
            ui->addsongsLabel->setText(tr("Playlist is at maximum size."));
        }
        // If playlist is not full set default labels and spinbox values
        else {
            ui->addtrksspinBox->setMaximum(s_MaxAvailableToAdd);
            if (s_MaxAvailableToAdd > 9) { ui->addtrksspinBox->setValue(10);}
            if (s_MaxAvailableToAdd < 10) {ui->addtrksspinBox->setValue(s_MaxAvailableToAdd);}
            ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd,'g', 3));
        }
    }
    if ((!s_bool_PlaylistExist)&&(s_bool_IsUserConfigSet)){
        if (Constants::kVerbose) std::cout << "Archsimian.cpp: Step 15. Default playlist not found. s_MaxAvailableToAdd at program launch is: "<< s_MaxAvailableToAdd << std::endl;
    }
    // End setup of UI
}

// Functions available from within the UI

void ArchSimian::on_addsongsButton_released(){
    if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Starting addSongs function." << std::endl;
    // Initialize progress bar settings
    ui->addsongsprogressBar->setVisible(true);
    ui->addsongsprogressBar->setRange(0, 100);
    ui->addsongsprogressBar->setValue(0);
    // Set messages and feedback to user during process
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    int numTracks = ui->addtrksspinBox->value(); // Sets the number of tracks the user selected to add (numtracks)
    // Reduce s_MaxAvailableToAdd quantity by number of tracks to be added (numTracks above)
    if (s_MaxAvailableToAdd > 0){
        s_MaxAvailableToAdd = s_MaxAvailableToAdd - numTracks;
    }
    // If tracks added makes s_MaxAvailableToAdd = 0, then dim the button and change label
    if (s_MaxAvailableToAdd < 1){
        s_MaxAvailableToAdd = 0;
        ui->addsongsButton->setEnabled(false);
        ui->addsongsLabel->setText(tr("Playlist is already at maximum size."));
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
    /* If adding the first song to an empty playlist, open an empty playlist (cleanedplaylist.txt) and
     and set an arbitrary rating code (6) for s_ratingNextTrack, before beginning */
    if (s_playlistSize == 0) {
        std::ofstream ofs; //open the cleanedplaylist file for writing with the truncate option to delete the content.
        ofs.open(appDataPathstr.toStdString()+"/cleanedplaylist.txt", std::ofstream::out | std::ofstream::trunc);
        ofs.close();
        {s_ratingNextTrack = 6;}        
    }
    std::ofstream songtext(appDataPathstr.toStdString()+"/songtext.txt",std::ios::app); // output file append mode for writing final song selections (UI display)
    // Determine the rating for the track selection if there are already tracks in the playlist
    if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Running ratingCodeSelected function before loop."<< std::endl;
    if (s_playlistSize > 0) {
        s_ratingNextTrack = ratingCodeSelected(s_ratingRatio3,s_ratingRatio4,s_ratingRatio5,s_ratingRatio6,
                                               s_ratingRatio7,s_ratingRatio8);
    }
    if (Constants::kVerbose) std::cout <<"on_addsongsButton_released: ratingCodeSelected function before loop completed. Result is: "<< s_ratingNextTrack <<
                                         ". Now starting loop (1047) to select tracks and add them to playlist..." <<std::endl;
    // Start loop for the number of tracks the user selected to add (numtracks)
    int currentprogress{0}; // set variable to output progress
    for (int i=0; i < numTracks; i++){
        if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Top of Loop (1050). Count: " <<i<<". Adding track "<< i + 1<<"." <<std::endl;
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
            if (Constants::kVerbose) std::cout << "on_addsongsButton_released (1078): Now running selectTrack for non-code-1 track selection (function selectTrack). Clearing s_selectedTrackPath" << std::endl;
            try {
                s_selectedTrackPath = "";
                selectTrack(s_ratingNextTrack,&s_selectedTrackPath,s_includeAlbumVariety); // Select track if not a code 1 selection
            }
            catch (const std::bad_alloc& exception) {
                std::cerr << "on_addsongsButton_released: bad_alloc detected: Maximum playlist length has been reached. Exiting program." << exception.what();
                i = numTracks;
                QMessageBox msgBox;
                QString msgboxtxt = "on_addsongsButton_released: Out of memory error (bad_alloc):failed during attempt to add tracks. Likely reason: "
                                    "Not enough available tracks found. Adjust factors on the frequency tab and restart.";
                msgBox.setText(msgboxtxt);
                msgBox.exec();
                if(s_mm4disabled == 0) {
                removeAppData("cleanlib.dsv");
                }
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
        getExcludedArtists(s_playlistSize);
        getExcludedArtistsRedux(s_playlistSize, int(s_histCount));
         // Recalc excluded artists
        if (Constants::kVerbose) std::cout << "on_addsongsButton_released: Running ratingCodeSelected function in loop."<< std::endl;
        s_ratingNextTrack = ratingCodeSelected(s_ratingRatio3,s_ratingRatio4,s_ratingRatio5,s_ratingRatio6,
                                               s_ratingRatio7,s_ratingRatio8); // Recalc rating selection
        if (Constants::kVerbose) std::cout<< "on_addsongsButton_released: ratingCodeSelected function in loop completed. Result: " << s_ratingNextTrack << ". Count "
                                                                                                                                                           "at end (1006) is now: "<< i<< std::endl;
        if (Constants::kVerbose) std::cout<< "on_addsongsButton_released: *****************************************************************" << std::endl;
        if (Constants::kVerbose) std::cout<< "on_addsongsButton_released: *************   Added track "<< i + 1<<".   ********************" << std::endl;
        if (Constants::kVerbose) std::cout<< "on_addsongsButton_released: *****************************************************************" << std::endl;
        currentprogress = currentprogress + int(100/(numTracks*0.75));
        ui->addsongsprogressBar->setValue(currentprogress);
    }
    if (s_MaxAvailableToAdd < 1){
        s_MaxAvailableToAdd = 0;
        ui->addsongsButton->setText("Add Songs");
        ui->addsongsButton->setEnabled(false);
        ui->addsongsLabel->setText(tr("Playlist is at maximum size."));
    }
    // If playlist is not full set default labels and spinbox values
    else {
        ui->addsongsButton->setText("Add Songs");
        ui->addtrksspinBox->setMaximum(s_MaxAvailableToAdd);
        if (s_MaxAvailableToAdd > 9) { ui->addtrksspinBox->setValue(10);}
        if (s_MaxAvailableToAdd < 10) {ui->addtrksspinBox->setValue(s_MaxAvailableToAdd);}
        ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd,'g', 3));
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
    if (s_MaxAvailableToAdd < 10) {ui->addtrksspinBox->setValue(s_MaxAvailableToAdd);}
    // Populate UI pane with a list of the tracks added and update playlist file for viewing
    ui->addsongsprogressBar->setVisible(false);
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

void ArchSimian::on_addsongsprogressBar_valueChanged(int value)
{
    if (value == 0){
        //ui->statusBar->showMessage("Adding songs to the playlist (takes time).",8000);
        ui->addsongsButton->setText("Adding songs...");
        ui->addsongsButton->setDisabled(true);
        //ui->addsongsprogressBar->setValue(+1);
    }
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
    ui->setmmdbButton->setEnabled(true);
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
    ui->windowsDriveLtrEdit->setEnabled(true);
}

void ArchSimian::on_windowsDriveLtrEdit_textChanged(const QString &arg1)
{
    m_prefs.s_WindowsDriveLetter = arg1;
    s_winDriveLtr = m_prefs.s_WindowsDriveLetter;
    if (s_winDriveLtr != ""){
        ui->saveConfigButton->setEnabled(true);
        ui->setFrqNextlabel->setText("After saving and restarting customize repeat setting using the Frequency tab.");
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
    m_prefs.s_androidpathname = settings.value("s_androidpathname","").toString();
    m_prefs.s_syncthingpathname = settings.value("s_syncthingpathname","").toString();
    m_prefs.s_mm4disabled = settings.value("s_mm4disabled", Constants::kUserDefaultMM4Disabled).toBool();
    m_prefs.s_audaciouslogenabled = settings.value("s_audaciouslogenabled", Constants::kAudaciouslogenabled).toBool();
    m_prefs.s_initalpostsettingslaunch = settings.value("s_initalpostsettingslaunch", Constants::k_initalpostsettingslaunch).toBool();
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
    settings.setValue("s_mm4disabled",m_prefs.s_mm4disabled);
    settings.setValue("s_androidpathname",m_prefs.s_androidpathname);
    settings.setValue("s_syncthingpathname",m_prefs.s_syncthingpathname);
    settings.setValue("s_audaciouslogenabled",m_prefs.s_audaciouslogenabled);
    settings.setValue("s_initalpostsettingslaunch",m_prefs.s_initalpostsettingslaunch);
}
void ArchSimian::closeEvent(QCloseEvent *event)
{
    event->ignore();
    if (s_noAutoSave == 1){
        saveSettings();
        event->accept();
    }
    if (s_noAutoSave == 0){
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
    ui->factor4doubleSpinBox->setEnabled(true);
    ui->factor5doubleSpinBox->setEnabled(true);
    ui->factor6doubleSpinBox->setEnabled(true);
    ui->factor7doubleSpinBox->setEnabled(true);
    m_prefs.s_daysTillRepeatCode3 = value;
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
    s_daysTillRepeatCode3 = m_prefs.s_daysTillRepeatCode3;
    s_DaysBeforeRepeatCode3 = s_yrsTillRepeatCode3 / Constants::kFractionOneDay;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    s_adjHoursCode3 = (1 / s_yrsTillRepeatCode3) * s_rCode3TotTime;
    s_ratingRatio3 = s_adjHoursCode3 / s_totAdjHours;
    selTrackLimitCodeRatingRatio = s_ratingRatio3;
    double tracksPerDay = (s_avgListeningRateInMins) / (s_AvgMinsPerSong);
    double interim1 = (tracksPerDay * selTrackLimitCodeRatingRatio * s_DaysBeforeRepeatCode3);
    int firstlimittest = int((selTrackLimitCodeTotTrackQty - interim1)/selTrackLimitCodeRatingRatio);
    int secondlimittest = int(tracksPerDay * s_DaysBeforeRepeatCode3 * 0.95);
    s_PlaylistLimit = std::min(firstlimittest,secondlimittest) - 20;
    s_MaxAvailableToAdd = s_PlaylistLimit;
    ui->newmaxavailLabel->setText(tr("Maximum (est.) tracks available is: ") + QString::number(s_MaxAvailableToAdd,'g', 3));
    ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd,'g', 3));
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
    ui->totratedtimefreqLabel->setText("Total time (in hours) is:               " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
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
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(argfact4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode4 = argfact4;
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
    ui->totratedtimefreqLabel->setText("Total time (in hours) is:               " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
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
    double test1{0.0};
    double test2{0.0};
    test1=((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
            ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
            ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
            ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
            ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
            ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime);
    test2=s_totalRatedTime;
    if(test1 < test2){
        m_prefs.s_repeatFactorCode4 = argfact4;
        ui->factor4doubleSpinBox->setEnabled(true);
        ui->factor5doubleSpinBox->setEnabled(true);
        ui->factor6doubleSpinBox->setEnabled(true);
        ui->factor7doubleSpinBox->setEnabled(true);
    }
    if(test1 > test2){
        ui->factor4doubleSpinBox->setEnabled(false);
        if ((ui->factor4doubleSpinBox->isEnabled() == false)&&(ui->factor5doubleSpinBox->isEnabled() == false)&&
                (ui->factor6doubleSpinBox->isEnabled() == false)&&(ui->factor7doubleSpinBox->isEnabled() == false)) {
            ui->factor3horizontalSlider->setValue((ui->factor3horizontalSlider->value()+1));
            ui->factor3IntTxtLabel->setNum(ui->factor3horizontalSlider->value());
        }
    }
}

void ArchSimian::on_factor5doubleSpinBox_valueChanged(double argfact5)
{
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    ui->factor5label->setText(QString::number(argfact5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode5 = argfact5;
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
    ui->totratedtimefreqLabel->setText("Total time (in hours) is:               " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
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
    double test1{0.0};
    double test2{0.0};
    test1=((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
            ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
            ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
            ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
            ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
            ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime);
    test2=s_totalRatedTime;
    if(test1 < test2){
        m_prefs.s_repeatFactorCode5 = argfact5;
        ui->factor4doubleSpinBox->setEnabled(true);
        ui->factor5doubleSpinBox->setEnabled(true);
        ui->factor6doubleSpinBox->setEnabled(true);
        ui->factor7doubleSpinBox->setEnabled(true);
    }
    if(test1 > test2){
        ui->factor5doubleSpinBox->setEnabled(false);
        if ((ui->factor4doubleSpinBox->isEnabled() == false)&&(ui->factor5doubleSpinBox->isEnabled() == false)&&
                (ui->factor6doubleSpinBox->isEnabled() == false)&&(ui->factor7doubleSpinBox->isEnabled() == false)) {
            ui->factor3horizontalSlider->setValue((ui->factor3horizontalSlider->value()+1));
            ui->factor3IntTxtLabel->setNum(ui->factor3horizontalSlider->value());
        }
    }
}

void ArchSimian::on_factor6doubleSpinBox_valueChanged(double argfact6)
{
    s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / Constants::kDaysInYear;
    sliderBaseVal3 = s_yrsTillRepeatCode3;
    ui->factor4label->setText(QString::number(m_prefs.s_repeatFactorCode4 * s_yrsTillRepeatCode3 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode4 = m_prefs.s_repeatFactorCode4;
    s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
    ui->factor5label->setText(QString::number(m_prefs.s_repeatFactorCode5 * s_yrsTillRepeatCode4 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode5 = m_prefs.s_repeatFactorCode5;
    s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
    ui->factor6label->setText(QString::number(argfact6 * s_yrsTillRepeatCode5 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode6 = argfact6;
    s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
    ui->factor7label->setText(QString::number(m_prefs.s_repeatFactorCode7 * s_yrsTillRepeatCode6 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_repeatFactorCode7 = m_prefs.s_repeatFactorCode7;
    s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    ui->totratedtimefreqLabel->setText("Total time (in hours) is:               " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
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
    double test1{0.0};
    double test2{0.0};
    test1=((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
            ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
            ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
            ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
            ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
            ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime);
    test2=s_totalRatedTime;
    if(test1 < test2){
        m_prefs.s_repeatFactorCode6 = argfact6;
        ui->factor4doubleSpinBox->setEnabled(true);
        ui->factor5doubleSpinBox->setEnabled(true);
        ui->factor6doubleSpinBox->setEnabled(true);
        ui->factor7doubleSpinBox->setEnabled(true);
    }
    if(test1 > test2){
        ui->factor6doubleSpinBox->setEnabled(false);
        if ((ui->factor4doubleSpinBox->isEnabled() == false)&&(ui->factor5doubleSpinBox->isEnabled() == false)&&
                (ui->factor6doubleSpinBox->isEnabled() == false)&&(ui->factor7doubleSpinBox->isEnabled() == false)) {
            ui->factor3horizontalSlider->setValue((ui->factor3horizontalSlider->value()+1));
            ui->factor3IntTxtLabel->setNum(ui->factor3horizontalSlider->value());
        }
    }
}

void ArchSimian::on_factor7doubleSpinBox_valueChanged(double argfact7)
{    
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
    s_repeatFactorCode7 = argfact7;
    s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
    ui->factor8label->setText(QString::number(m_prefs.s_repeatFactorCode8 * s_yrsTillRepeatCode7 * s_dateTranslation,'g', 3) + dateTransTextVal);
    s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
    ui->totratedtimefreqLabel->setText("Total time (in hours) is:               " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
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

    double test1{0.0};
    double test2{0.0};
    test1=((1 / s_yrsTillRepeatCode3) * s_rCode3TotTime) +
            ((1 / s_yrsTillRepeatCode4) * s_rCode4TotTime) +
            ((1 / s_yrsTillRepeatCode5) * s_rCode5TotTime) +
            ((1 / s_yrsTillRepeatCode6) * s_rCode6TotTime) +
            ((1 / s_yrsTillRepeatCode7) * s_rCode7TotTime) +
            ((1 / s_yrsTillRepeatCode8) * s_rCode8TotTime);
    test2=s_totalRatedTime;
    if(test1 < test2){
        m_prefs.s_repeatFactorCode7 = argfact7;
        ui->factor4doubleSpinBox->setEnabled(true);
        ui->factor5doubleSpinBox->setEnabled(true);
        ui->factor6doubleSpinBox->setEnabled(true);
        ui->factor7doubleSpinBox->setEnabled(true);
    }
    if(test1 > test2){
        ui->factor7doubleSpinBox->setEnabled(false);
        if ((ui->factor4doubleSpinBox->isEnabled() == false)&&(ui->factor5doubleSpinBox->isEnabled() == false)&&
                (ui->factor6doubleSpinBox->isEnabled() == false)&&(ui->factor7doubleSpinBox->isEnabled() == false)) {
            ui->factor3horizontalSlider->setValue((ui->factor3horizontalSlider->value()+1));
            ui->factor3IntTxtLabel->setNum(ui->factor3horizontalSlider->value());
        }
    }
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
    ui->totratedtimefreqLabel->setText("Total time (in hours) is:               " + QString::fromStdString(std::to_string(int(s_totalRatedTime))));
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
    saveSettings();
    ui->statusBar->showMessage("Saved Archsimian-modified playlist in Windows directory format",4000);
}

void ArchSimian::on_actionExit_triggered()
{
    if (!s_noAutoSave){
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: on_actionExit_triggered. noAutoSave is set to false, meaning settings are saved automatically on exit. " << std::endl;}
        saveSettings();
        std::ofstream ofs; //open the cleanedplaylist file for writing with the truncate option to delete the content.
        ofs.open(appDataPathstr.toStdString()+"/cleanedplaylist.txt", std::ofstream::out | std::ofstream::trunc);
        ofs.close();
        s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: on_actionExit_triggered. cleanedplaylist should be zero now: "<< s_playlistSize << std::endl;}
        qApp->quit();
    }
    if (s_noAutoSave){
        if (Constants::kVerbose){std::cout << "Archsimian.cpp: on_actionExit_triggered. noAutoSave is set to true, meaning settings are not saved automatically on exit. " << std::endl;}
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
    QMessageBox::about(this,tr("ArchSimian") ,tr("\nArchSimian v.1.05"
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
        // Recalculate historical count (from step 12)
        s_histCount = int(s_SequentialTrackLimit - s_playlistSize);
        if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered: Add/change playlist.. s_histCount is: "<< s_histCount << std::endl;}
        // Update excluded artists (from step 13) by running function getExcludedArtists() which also recreates ratedabbr2, and check code1 stats
        if (Constants::kVerbose){std::cout << "on_actionOpen_Playlist_triggered: Processing artist stats and excluded artists list. Creating temporary "
                                              "database (ratedabbr2.txt)." << std::endl;}
        getExcludedArtists(s_playlistSize);
        getExcludedArtistsRedux(s_playlistSize, int(s_histCount));
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
        s_OpenPlaylistLimit = s_PlaylistLimit;
        s_MaxAvailableToAdd = s_OpenPlaylistLimit - s_playlistSize;
        if (s_MaxAvailableToAdd < 1){
            s_MaxAvailableToAdd = 0;
            ui->addsongsButton->setEnabled(false);
            ui->addsongsLabel->setText(tr("Playlist is at maximum size."));
        }
        // If playlist is not full set default labels and spinbox values
        else {
            ui->addtrksspinBox->setMaximum(s_MaxAvailableToAdd);
            if (s_MaxAvailableToAdd > 9) { ui->addtrksspinBox->setValue(10);}
            if (s_MaxAvailableToAdd < 10) {ui->addtrksspinBox->setValue(s_MaxAvailableToAdd);}
            ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd,'g', 3));
        }
        // Finalize playlist loading
        ui->currentplsizeLabel->setText(tr("Current playlist size is ") + QString::number(s_playlistSize)+tr(" tracks, "));
        ui->playlistdaysLabel->setText(tr("and playlist length in listening days is ") + QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
        std::string MMdbDate;
        if (!s_mm4disabled) {MMdbDate = getMMdbDate(s_mmBackupDBDir);}
        std::string LastTableDate = getLastTableDate();
        ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(MMdbDate)+
                                       tr(", Library date: ")+ QString::fromStdString(LastTableDate));
        if (s_mm4disabled){
        ui->updatestatusLabel->setText(tr("MM.DB date: Disabled, Library date: ")+ QString::fromStdString(LastTableDate));
        }
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
    // Update excluded artists  (from step 13) by running function getExcludedArtists() which also recreates ratedabbr2, and check code1 stats
    if (Constants::kVerbose){std::cout << "on_actionNew_Playlist_triggered: Running getPlaylist." << std::endl;}
    getPlaylist(s_defaultPlaylist, s_musiclibrarydirname, s_musiclibshortened, s_topLevelFolderExists);
    s_bool_PlaylistSelected = true;
    // Get playlist size  (from step 11)
    s_playlistSize = cstyleStringCount(appDataPathstr.toStdString()+"/cleanedplaylist.txt");
    if (s_playlistSize == 1) s_playlistSize = 0;
    if (Constants::kVerbose){std::cout << "on_actionNew_Playlist_triggered: Add/change playlist. s_playlistSize is: "<< s_playlistSize << std::endl;}
    // Recalculate historical count  (from step 12). Since it is a new playlist, set s_histCount = s_SequentialTrackLimit.
    s_histCount = int(s_SequentialTrackLimit);
    if (Constants::kVerbose){std::cout << "on_actionNew_Playlist_triggered: Add/change playlist. s_histCount is: "<< s_histCount << std::endl;}
    // Update excluded artists  (from step 13) by running function getExcludedArtists() which also recreates ratedabbr2, and check code1 stats
    if (Constants::kVerbose){std::cout << "on_actionNew_Playlist_triggered: Running getExcludedArtists." << std::endl;}
    getExcludedArtists(s_playlistSize);
    getExcludedArtistsRedux(s_playlistSize, int(s_histCount));
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
    // Set s_MaxAvailableToAdd to s_OpenPlaylistLimit without adjustment since new playlist is empty
    s_OpenPlaylistLimit = s_PlaylistLimit;
    s_MaxAvailableToAdd = s_OpenPlaylistLimit;
    ui->addtrksspinBox->setMaximum(s_MaxAvailableToAdd);
    if (s_MaxAvailableToAdd > 9) { ui->addtrksspinBox->setValue(50);}
    ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd,'g', 3));
    // Finalize playlist loading
    ui->setgetplaylistLabel->setText("Selected: " + s_defaultPlaylist);
    ui->currentplsizeLabel->setText(tr("Current playlist size is ") + QString::number(s_playlistSize)+tr(" tracks, "));
    ui->playlistdaysLabel->setText(tr("and playlist length in listening days is ") + QString::number(s_playlistSize/(s_avgListeningRateInMins / s_AvgMinsPerSong),'g', 3));
    ui->addtrksspinBox->setValue(30);
    ui->addsongsLabel->setText(tr(" tracks to selected playlist. May add a max of: ") + QString::number(s_MaxAvailableToAdd,'g', 3));
    std::string MMdbDate;
    if (!s_mm4disabled) {MMdbDate = getMMdbDate(s_mmBackupDBDir);}
    std::string LastTableDate = getLastTableDate();
    ui->updatestatusLabel->setText(tr("MM.DB date: ") + QString::fromStdString(MMdbDate)+
                                   tr(", Library date: ")+ QString::fromStdString(LastTableDate));
    if (s_mm4disabled){
        ui->updatestatusLabel->setText(tr("MM.DB date: Disabled, Library date: ")+ QString::fromStdString(LastTableDate));
    }
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
    ui->autosavecheckBox->checkState();
    //s_noAutoSave = autosave;
    m_prefs.s_noAutoSave = autosave;
    if (ui->autosavecheckBox->checkState() == 2){
        m_prefs.s_noAutoSave = true;
        QWidget::repaint();
    }
    if (ui->autosavecheckBox->checkState() == 0){
        m_prefs.s_noAutoSave = false;
        QWidget::repaint();
    }
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
    ui->setFrqNextlabel->setText("");
    s_initalpostsettingslaunch = true;
    m_prefs.s_initalpostsettingslaunch =s_initalpostsettingslaunch;
    saveSettings();
    qApp->quit();
}



//void ArchSimian::on_playlistLimitSlider_valueChanged(int value)
//{
// Adjustment for track limit percentage. Set variable by dividing slider value by 100.
//}


void ArchSimian::on_actionIterate_tag_triggered()
{
updateChangedTagRatings();
}

void ArchSimian::on_actionGet_Last_Played_Dates_triggered()
{
    getLastPlayedDates(s_androidpathname);
}

void ArchSimian::on_actionDate_Conversion_triggered()
{
    //logdateconversion();
}

void ArchSimian::on_actionSyncPlaylist_triggered()
{
    syncPlaylistWithSyncthing();
}

void ArchSimian::on_actionRemove_mp3s_triggered()
{
    removeMP3sSyncthing();
}

void ArchSimian::on_mmdisabledradioButton_clicked()
{
    s_mm4disabled = true;
    m_prefs.s_mm4disabled = s_mm4disabled;
    saveSettings();
    //    ui->updateASDBButton->setDisabled(false);
    ui->syncthingButton->setDisabled(false);
    ui->selectAndroidDeviceButton->setDisabled(false);
    ui->updateASDBButton->setDisabled(false);
    ui->enableAIMPOnlyradioButton->setDisabled(false);
    ui->enableAudaciousLogButton->setDisabled(false);
    ui->syncPlaylistButton->setDisabled(false);
    ui->updateratingsButton->setDisabled(false);
    ui->mainQTabWidget->setTabEnabled(5, true);
    ui->syncTab->setEnabled(true);

    if (Constants::kVerbose){std::cout << "s_mm4disabled changed to true: "<<s_mm4disabled << std::endl;}
    ui->statusBar->showMessage("Changed database from MediaMonkey to ArchSimian.",4000);
}

void ArchSimian::on_mmenabledradioButton_2_clicked()
{
    s_mm4disabled = false;
    m_prefs.s_mm4disabled = s_mm4disabled;
    saveSettings();
    ui->syncthingButton->setDisabled(true);
    ui->selectAndroidDeviceButton->setDisabled(true);
    ui->updateASDBButton->setDisabled(true);
    ui->enableAIMPOnlyradioButton->setDisabled(true);
    ui->enableAudaciousLogButton->setDisabled(true);
    ui->syncPlaylistButton->setDisabled(true);
    ui->updateratingsButton->setDisabled(true);
    ui->mainQTabWidget->setTabEnabled(5, false);
    ui->syncTab->setEnabled(false);
    if (Constants::kVerbose){std::cout << "s_mm4disabled changed to false: "<<s_mm4disabled << std::endl;}
    ui->statusBar->showMessage("Changed database from ArchSimian to MediaMonkey.",4000);
}

void ArchSimian::on_selectAndroidDeviceButton_clicked()
{
    QFileDialog selectAndroidDeviceButton;
    selectAndroidDeviceButton.setFileMode(QFileDialog::Directory);
    selectAndroidDeviceButton.setOption(QFileDialog::ShowDirsOnly);
    s_androidpathname= QFileDialog::getExistingDirectory(
                this,
                tr("Select Path of Android Device"),
                "/"
                );
    ui->androiddevicebuttonlabel->setText(QString(s_androidpathname));
    if ((s_androidpathname == "" ) || (s_syncthingpathname == "")) { // If either of the 2 sync paths have not been established dim two action buttons
                ui->updateASDBButton->setDisabled(true);
                ui->syncPlaylistButton->setDisabled(true);
    }
    if ((s_androidpathname != "" ) && (s_syncthingpathname != "")) { // If both of the 2 sync paths have been established enable two action buttons
                ui->updateASDBButton->setDisabled(false);
                ui->syncPlaylistButton->setDisabled(false);
    }
    // Write description note and directory configuration to archsimian.conf
    m_prefs.s_androidpathname = s_androidpathname;
    saveSettings();
    ui->statusBar->showMessage("Saved device location in ArchSimian.",4000);
}

void ArchSimian::on_updateASDBButton_clicked()
{
    //ui->updateASDBprogressBar->setVisible(true);
    //ui->updateASDBprogressBar->setRange(0, 100);
    //ui->updateASDBprogressBar->setValue(10);
    if (Constants::kVerbose){std::cout << "on_updateASDBButton_clicked: Starting getLastPlayedDates."<< std::endl;}
    getLastPlayedDates(s_androidpathname); // First, poll the AIMP log and get last played dates
    ui->updateASDBButton->toggle();
    if (s_audaciouslogenabled == true) { // If Audacious logging enabled, process its play history
        //ui->updateASDBprogressBar->setValue(50);
        if (Constants::kVerbose){std::cout << "on_updateASDBButton_clicked: Starting syncAudaciousLog."<< std::endl;}
        syncAudaciousLog();
    }
    ui->updateASDBprogressBar->setValue(80);
    if (Constants::kVerbose){std::cout << "on_updateASDBButton_clicked: Starting updateCleanLibDates."<< std::endl;}
    updateCleanLibDates(); // Update cleanlib.dsv wih new dates
    //ui->updateASDBButton->setText("Reprocessing stats...please wait");
    //if (Constants::kVerbose){std::cout << "on_updateASDBButton_clicked: Starting getDBStats."<< std::endl;}
    /*getDBStats(&s_rCode0TotTrackQty,&s_rCode0MsTotTime,&s_rCode1TotTrackQty,&s_rCode1MsTotTime,
               &s_rCode3TotTrackQty,&s_rCode3MsTotTime,&s_rCode4TotTrackQty,&s_rCode4MsTotTime,
               &s_rCode5TotTrackQty,&s_rCode5MsTotTime,&s_rCode6TotTrackQty,&s_rCode6MsTotTime,
               &s_rCode7TotTrackQty,&s_rCode7MsTotTime,&s_rCode8TotTrackQty,&s_rCode8MsTotTime,
               &s_SQL10TotTimeListened,&s_SQL10DayTracksTot,&s_SQL20TotTimeListened,
               &s_SQL20DayTracksTot,&s_SQL30TotTimeListened,&s_SQL30DayTracksTot,&s_SQL40TotTimeListened,
               &s_SQL40DayTracksTot,&s_SQL50TotTimeListened,&s_SQL50DayTracksTot,&s_SQL60TotTimeListened,
               &s_SQL60DayTracksTot);*/
    //getExcludedArtists(s_playlistSize);
    // Need to reprocess functions associated with a cleanlib.dsv change.
    std::string LastTableDate = getLastTableDate();
    ui->updateASDBButton->setText("Update ArchSimian Database");
    ui->updateASDBButton->setDisabled(false); // Reenable button after updating
    ui->updatestatusLabel->setText(tr("MM.DB date: Disabled, Library date: ")+ QString::fromStdString(LastTableDate));
    ui->statusBar->showMessage("Completed getting lastplayed dates and updating Archsimian.",8000);
    //ui->updateASDBprogressBar->setVisible(false);
    QFile ratingupdate(appDataPathstr+"/syncdisplay.txt");
    if(!ratingupdate.open(QIODevice::ReadOnly))
        QMessageBox::information(nullptr,"info",ratingupdate.errorString());
    QTextStream in(&ratingupdate);
    ui->updateDBtextBrowser->setText(in.readAll());
    ui->statusBar->showMessage("Finished updating play history to the database",8000);
}

void ArchSimian::on_updateASDBprogressBar_valueChanged(int value)
{
    if (value == 10){
        //ui->statusBar->showMessage("Updating ratings by scanning library files for changed tags (takes time).",80000);
        ui->updateASDBButton->setText("Getting lastplayed dates...");
        ui->updateASDBButton->setDisabled(true);
        ui->updateASDBprogressBar->setValue(+20);
    }
    if (value == 50){
        ui->updateASDBButton->setText("Getting Audacious dates...");
        ui->updateASDBprogressBar->setValue(+15);
    }
    if (value == 80){
        ui->updateASDBButton->setText("Updating database......");
        ui->updateASDBprogressBar->setValue(+20);
    }
}

void ArchSimian::on_actionUpdateLastPlayed_triggered() // already included on button for on_updateASDBButton_clicked() above - remove from menu
{
    //updateCleanLibDates();
    getExcludedArtists(s_playlistSize);
    getExcludedArtistsRedux(s_playlistSize, int(s_histCount));
}

void ArchSimian::on_actionExport_Playlist_to_Linux_triggered()
{
    removeLinuxPlaylistFile(); // First delete old playlist file
    exportPlaylistToLinux(); // Export new one.
    ui->statusBar->showMessage("Playlist named cleanedplaylist.m3u with linux path exported to home folder.",4000);
}

void ArchSimian::on_syncPlaylistButton_clicked()
{
    syncPlaylistWithSyncthing();
}

void ArchSimian::on_syncthingButton_clicked()
{
    QFileDialog selectsyncthingButton;
    selectsyncthingButton.setFileMode(QFileDialog::Directory);
    selectsyncthingButton.setOption(QFileDialog::ShowDirsOnly);
    s_syncthingpathname= QFileDialog::getExistingDirectory(
                this,
                tr("Select Path of Syncthing shared folder"),
                "/"
                );
    ui->syncthinglabel->setText(QString(s_syncthingpathname));    
    m_prefs.s_syncthingpathname = s_syncthingpathname; // Write description note and directory configuration to archsimian.conf
    saveSettings();
    if ((s_androidpathname == "" ) || (s_syncthingpathname == "")) { // If either of the 2 sync paths have not been established dim two action buttons
                ui->updateASDBButton->setDisabled(true);
                ui->syncPlaylistButton->setDisabled(true);
    }
    if ((s_androidpathname != "" ) && (s_syncthingpathname != "")) { // If both of the 2 sync paths have been established enable two action buttons
                ui->updateASDBButton->setDisabled(false);
                ui->syncPlaylistButton->setDisabled(false);
    }
    ui->statusBar->showMessage("Saved folder location in ArchSimian.",4000);
}

void ArchSimian::on_actionsyncAudaciousLog_triggered() // Need to add to on_updateASDBButton_clicked() process if enabled separately
{
    syncAudaciousLog();
}

void ArchSimian::on_enableAudaciousLogButton_clicked()
{
    s_audaciouslogenabled = true;
    ui->enableAudaciousLogButton->setChecked(true);
    m_prefs.s_audaciouslogenabled = s_audaciouslogenabled;
    saveSettings();
    if (Constants::kVerbose){std::cout << "s_audaciouslogenabled changed to true: "<<s_audaciouslogenabled << std::endl;}
    ui->statusBar->showMessage("Enabled play history logging from Audacious.",4000);
}

void ArchSimian::on_enableAIMPOnlyradioButton_clicked()
{
    s_audaciouslogenabled = false;
    ui->enableAIMPOnlyradioButton->setChecked(true);
    m_prefs.s_audaciouslogenabled = s_audaciouslogenabled;
    saveSettings();
    if (Constants::kVerbose){std::cout << "s_audaciouslogenabled changed to false: "<<s_audaciouslogenabled << std::endl;}
    ui->statusBar->showMessage("Disabled play history logging from Audacious.",4000);
}

void ArchSimian::on_updateratingsButton_clicked()
{    
    ui->updateTagsprogressBar->setVisible(true);
    ui->updateTagsprogressBar->setValue(0);
    statusLabel->setText("Update ratings");
    ui->statusBar->showMessage("Scanning library files for changed tags (takes time)...",30000);
    updateChangedTagRatings();
    statusLabel->setText("");
    ui->updateTagsprogressBar->setVisible(false);
    ui->updateratingsButton->setText("Update Ratings from Tags");
    ui->updateratingsButton->setDisabled(false);
    QFile ratingupdate(appDataPathstr+"/syncdisplay.txt");
    if(!ratingupdate.open(QIODevice::ReadOnly))
        QMessageBox::information(nullptr,"info",ratingupdate.errorString());
    QTextStream in(&ratingupdate);
    ui->updateDBtextBrowser->setText(in.readAll());
    ui->statusBar->showMessage("Finished updating tag ratings to the database",8000);
}

void ArchSimian::on_updateTagsprogressBar_valueChanged(int value)
{
    if (value == 0){
    ui->updateratingsButton->setText("Updating...please wait");
    ui->updateratingsButton->setDisabled(true);
    ui->updateTagsprogressBar->setValue(+1);
    }
    if (value == 1){
        ui->statusBar->showMessage("Scanning library files for changed tags (takes time)...",80000);
    }
}

void ArchSimian::on_freqconfigButton_clicked()
{
    ui->setFrqNextlabel->setText("");
    s_initalpostsettingslaunch = false;
    m_prefs.s_initalpostsettingslaunch =s_initalpostsettingslaunch;
    saveSettings();
    qApp->quit();
}
