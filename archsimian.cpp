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
#include "getcleanlib.h"
#include "getlibvariables.h"
#include "getrated.h"
#include "getcleanlib.h"
#include "getplaylist.h"
#include "getartistadjustedount.h"
#include "addintervalvalues.h"
#include "getartistexcludes.h"
#include "getartistexcludes2.h"
#include "getartistexcludes3.h"
#include "getartistexcludes4.h"
#include "getartistexcludes5.h"
#include "writesqlfile.h"
#include "ratingvariance.h"

template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N]) {//Function to execute command line with parameters
    assert((N > 0) && (argv[N - 1] == nullptr));
    return execvp(file, const_cast<char* const*>(argv));
}

// VARIABLE DECLARATIONS
static std::string s_mmbackupdbdirname;
static std::string s_musiclibrarydirname;
static std::string s_mmbackuppldirname;
static std::string s_selectedplaylist;
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
static double s_ratingRatio3;
static double s_ratingRatio4;
static double s_ratingRatio5;
static double s_ratingRatio6;
static double s_ratingRatio7;
static double s_ratingRatio8;
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

static double s_listeningRate;
static int s_totalRatedQty;
static double s_totalRatedTime;
static int s_totalLibQty;
static double s_DaysBeforeRepeatCode3;
static double s_totHrsLast60Days;
static double s_totalAdjRatedQty;

static unsigned long s_playlistSize;

ArchSimian::ArchSimian(QWidget *parent) :    
    QMainWindow(parent),
    ui(new Ui::ArchSimian)
{
    // User configuration: set default state to "false" for user config reset buttons
    ui->setupUi(this);
    ui->setlibraryButtonReset->setVisible(false);
    ui->setmmplButtonReset->setVisible(false);
    ui->setmmdbButtonReset->setVisible(false);

    //Check whether the configuration file currently has any data in it
    std::streampos size;
    char * memblock;
    std:: ifstream file ("archsimian.conf", std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        memblock = new char [size];
        file.seekg (0, std::ios::beg);
        file.read (memblock, size);
        file.close();
        delete[] memblock;
    }
    else std::cout << "archsimian.cpp: Unable to open configuration file";

    //If configuration has already been set, populate the ui labels accordingly
    if (size != 0)
    {
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
        bool needUpdate = isLibRefreshNeeded(); // function isLibRefreshNeeded() is from dependents.cpp
        if (needUpdate == 0)
        {
            ui->updatestatusLabel->setText(tr("MM.DB was last updated: "));
            // dim update library button
            ui->refreshdbButton->setEnabled(false);
        }
    }

    else {  // Otherwise, configuration has not been set. Load instructions for user to locate and set config
        // Initially, only the first of three config buttons is activated. Build the file in sequence.
        ui->setlibrarylabel->setText(tr("Select the base directory of "
                                        "your music library"));
        ui->setlibraryButton->setEnabled(true);
        ui->setlibraryButtonReset->setVisible(false);
        ui->setmmpllabel->setText(tr("Select the shared Windows directory"
                                     " where you stored the backup playlists from MediaMonkey"));
        ui->setmmplButton->setEnabled(false);
        ui->setmmplButtonReset->setVisible(false);
        ui->setmmdblabel->setText(tr("Select the shared Windows directory"
                                     " where you stored the MediaMonkey database backup file"));
        ui->setmmdbButton->setEnabled(false);
        ui->setmmdbButtonReset->setVisible(false);
        ui->setgetplaylistLabel->setText(tr("Select playlist for adding tracks"));
    }

    // Set variable for customArtistID, either dir tree (0) or custom groupings (1)
    //    bool customArtistID{1};
    //
    //*********************************************************************
    // CHECK DATE OF LAST MM.DB BACKUP AGAINST THE RATEDLIB.DSV (IF EXISTS)
    //*********************************************************************
    // Check to see if the ratedlib.dsv file already exists
    bool needUpdate = 1;//isLibRefreshNeeded(); // function isLibRefreshNeeded() is from dependents.cpp

    if (needUpdate == 1) // bool needUpdate: 1 means refresh DB, 0 means skip unessential
    {
        ui->refreshdbButton->setEnabled(true);
        ui->updatestatusLabel->setText(tr("MM.DB has been backed up. Refresh data."));

        // add

       // const QString title("New backup identified");
       // const QString message("Getting MM.DB table and compiling stats");
       // int millisecondsTimeoutHint = 10000;
        //if (trayIcon->isVisible()) {
        //QSystemTrayIcon ret2;
        //ret2.setVisible(1);
        //ret2.show("Getting MM.DB table and compiling stats");
        //ret2.showMessage("Getting MM.DB table and compiling stats");
        //QMessageBox ret;
        //ret.information(this,tr("New backup identified"),tr("Getting MM.DB table and compiling stats"));
        //ret.show();
    }
   //else {
   //     ui->refreshdbButton->setEnabled(false);
   // }
    if (needUpdate == 0) { // bool needUpdate: 1 means refresh DB, 0 means skip unessential - still need getCleanLib for stats
        //ui->refreshdbButton->setEnabled(false);
        // Launch function to fix dir path for linux and obtain the values for statistical variables, creates cleanlib.dsv
        getCleanLib(&s_rCode0TotTrackQty,&s_rCode0MsTotTime,&s_rCode1TotTrackQty,&s_rCode1MsTotTime,&s_rCode3TotTrackQty,&s_rCode3MsTotTime,
                    &s_rCode4TotTrackQty,&s_rCode4MsTotTime, &s_rCode5TotTrackQty,&s_rCode5MsTotTime,&s_rCode6TotTrackQty,
                    &s_rCode6MsTotTime, &s_rCode7TotTrackQty, &s_rCode7MsTotTime,&s_rCode8TotTrackQty, &s_rCode8MsTotTime,
                    &s_SQL10TotTimeListened, &s_SQL10DayTracksTot, &s_SQL20TotTimeListened,&s_SQL20DayTracksTot, &s_SQL30TotTimeListened,
                    &s_SQL30DayTracksTot,&s_SQL40TotTimeListened, &s_SQL40DayTracksTot, &s_SQL50TotTimeListened,
                    &s_SQL50DayTracksTot, &s_SQL60TotTimeListened, &s_SQL60DayTracksTot);

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

    //Print results to console (for later program integration tasks (TBD)
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
    //std::cout << "Sequential Track Limit Factor - s_STLF : "<< s_STLF << std::endl;
    std::cout << "Now processing statistics. This will take a few seconds..."<< std::endl;

    //4
    std::fstream filestr;
    filestr.open ("cleanlib.dsv");
    if (filestr.is_open()) {filestr.close();
        std::cout << "File cleanlib.dsv successfully created." << std::endl;
        //remove("libtable.dsv");
    }
    else {
        std::cout << "Error opening cleanlib.dsv file" << std::endl;}
    std::vector<std::string> plStrings;
    //6
    // Using the function getPlaylistVect (also from getplaylist.cpp), load cleanedplaylist.txt into a vector plStrings
    getPlaylistVect("cleanedplaylist.txt", plStrings);
    s_playlistSize = plStrings.size();
    std::cout << "getPlaylistVect completed." << std::endl;
    plStrings.shrink_to_fit();
    // Using libtable.dsv from parent process create rated.dsv with random lastplayed dates created for
    // unplayed (but rated or new need-to-be-rated tracks with no play history); also adds playlist position number to Custom1 field
    // from the function getPlaylistVect
    //7
    //Section beyond initial program setup
    // Run function getArtistExcludes to populate a list of artists currently unavailable
    getArtistExcludes();
    std::cout << "getArtistExcludes completed." << std::endl;
    // Run function getArtistExcludes2 to populate a list of artists currently unavailable
    getArtistExcludes2();
    std::cout << "getArtistExcludes2 completed." << std::endl;
    std::cout << "Current playlist size is: "<< s_playlistSize << std::endl;
    //unsigned long histCount = s_SequentialTrackLimit - s_playlistSize;
    static long s_histCount = long(s_SequentialTrackLimit) - long(s_playlistSize);
    std::cout << "History count for calculating additional artist excludes: "<< s_histCount << std::endl;
    getExtendedExcludes(&s_histCount,&s_playlistSize);
    std::cout << "getExtendedExcludes completed." << std::endl;
    appendExcludes();
    std::cout << "appendExcludes completed." << std::endl;
    fixsort();
    int ratingNextTrack;
    ratingNextTrack = ratingCodeSelected(&s_ratingRatio3,&s_ratingRatio4,&s_ratingRatio5,&s_ratingRatio6,&s_ratingRatio7,&s_ratingRatio8);
    std::cout << "Rating for the next track is " << ratingNextTrack << std::endl;
    std::cout << "done!" << std::endl;
    }
}

void ArchSimian::on_addsongsButton_clicked(){

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
    // First reset variables
    s_totalLibQty = 0;
    s_totalRatedQty = 0;
    s_totalRatedTime = 0;
    s_listeningRate = 0;
    //
    // Update label with updating status and dim button
    ui->refreshdbButton->setEnabled(false);

    // Run function to create and write a sql file to user's home directory
    writeSQLFile();
    sleep(1);

    pid_t c_pid;// Create fork object; child to get database table into a dsv file, then child to open that table only
    // after it finishes getting written, not before.
    c_pid = fork(); // Run fork function
    int status; // For status of pid process
    //1
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
    //2
    else if (c_pid > 0){  // Parent process starts here. Write from libtable.dsv and gather stats
        // First, reopen libtable.dsv, clean track paths, and output to cleanlib.dsv
        //Check to ensure the file has finshed being written

        if( (c_pid = wait(&status)) < 0){
            perror("wait");
            _exit(1);
        }

        // Launch function to fix dir path for linux and obtain the values for statistical variables, creates cleanlib.dsv
        getCleanLib(&s_rCode0TotTrackQty,&s_rCode0MsTotTime,&s_rCode1TotTrackQty,&s_rCode1MsTotTime,&s_rCode3TotTrackQty,&s_rCode3MsTotTime,
                    &s_rCode4TotTrackQty,&s_rCode4MsTotTime, &s_rCode5TotTrackQty,&s_rCode5MsTotTime,&s_rCode6TotTrackQty,
                    &s_rCode6MsTotTime, &s_rCode7TotTrackQty, &s_rCode7MsTotTime,&s_rCode8TotTrackQty, &s_rCode8MsTotTime,
                    &s_SQL10TotTimeListened, &s_SQL10DayTracksTot, &s_SQL20TotTimeListened,&s_SQL20DayTracksTot, &s_SQL30TotTimeListened,
                    &s_SQL30DayTracksTot,&s_SQL40TotTimeListened, &s_SQL40DayTracksTot, &s_SQL50TotTimeListened,
                    &s_SQL50DayTracksTot, &s_SQL60TotTimeListened, &s_SQL60DayTracksTot);
    }
    else { // if (c_pid < 0) error check: The return of fork() is negative
        perror("fork failed");
        _exit(2); //exit failure, hard
    }
    // Need stat calculations for both needUpdate states
    // Convert variables from milliseconds to hours
    //  Total time in hours per rating code
    //static int s_rCode0TotTime = (s_rCode0MsTotTime/60000)/60;
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
    //static double s_totHrsLast60Days = s_SQL10TotTimeListened + s_SQL20TotTimeListened + s_SQL30TotTimeListened + s_SQL40TotTimeListened
    //        + s_SQL50TotTimeListened + s_SQL60TotTimeListened; //Total listened hours in the last 60 days
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
    //static double s_totalAdjRatedQty = (s_yrsTillRepeatCode3factor * s_rCode3TotTrackQty)+(s_yrsTillRepeatCode4factor * s_rCode4TotTrackQty)
    //        + (s_yrsTillRepeatCode5factor * s_rCode5TotTrackQty) +(s_yrsTillRepeatCode6factor * s_rCode6TotTrackQty)
    //        +(s_yrsTillRepeatCode7factor * s_rCode7TotTrackQty) + (s_yrsTillRepeatCode8factor * s_rCode8TotTrackQty);
    std::fstream filestr;
    filestr.open ("cleanlib.dsv");
    if (filestr.is_open()) {filestr.close();
        std::cout << "File cleanlib.dsv successfully created." << std::endl;
        //remove("libtable.dsv");
    }
    else {
        std::cout << "Error opening cleanlib.dsv file" << std::endl;}

        removeSQLFile(); // Run function to delete sql file from user's home directory after completion
        getPlaylist(); // Using the function getPlaylist, correct paths from windows to linux, then save to cleanedplaylist.txt
        std::cout << "getPlaylist completed." << std::endl;

    std::vector<std::string> plStrings;
    //6
    // Using the function getPlaylistVect (also from getplaylist.cpp), load cleanedplaylist.txt into a vector plStrings
    getPlaylistVect("cleanedplaylist.txt", plStrings);
    s_playlistSize = plStrings.size();
    std::cout << "getPlaylistVect completed." << std::endl;
    plStrings.shrink_to_fit();
    // Using libtable.dsv from parent process create rated.dsv with random lastplayed dates created for
    // unplayed (but rated or new need-to-be-rated tracks with no play history); also adds playlist position number to Custom1 field
    // from the function getPlaylistVect
    //7
      // bool needUpdate: 1 means refresh DB, 0 means skip unessential
        getRatedTable();
        std::cout << "getRatedTable completed." << std::endl;
        std::fstream filestr2;
        filestr2.open ("rated.dsv");
        if (filestr2.is_open()) {
            filestr2.close();
            std::cout << "File rated.dsv successfully created. Deleting cleanlib.dsv." << std::endl;
            remove("cleanlib.dsv");}
        else {std::cout << "Error opening rated.dsv file before deleting cleanlib.dsv" << std::endl;}

        // To set up artist-related data, determine the identifier for artists (add selector to GUI configuration)
        //bool customArtistID = 1; // manually set to true (means use Custom 2 for artist)
        //8
        // Using the function getArtistAdjustedCount, use rated.dsv to generate unique artist list, count tracks, calculate adjusted tracks,
        // calculate factors, calculate repeat intervals, then write the artist values to
        getArtistAdjustedCount(&s_yrsTillRepeatCode3factor,&s_yrsTillRepeatCode4factor,&s_yrsTillRepeatCode5factor,
                               &s_yrsTillRepeatCode6factor,&s_yrsTillRepeatCode7factor,&s_yrsTillRepeatCode8factor,
                               &s_rCode3TotTrackQty,&s_rCode4TotTrackQty,&s_rCode5TotTrackQty,
                               &s_rCode6TotTrackQty,&s_rCode7TotTrackQty,&s_rCode8TotTrackQty);

        std::cout << "getArtistAdjustedCount completed." << std::endl;
        //9
        // Run function addIntervalValues to baseline the variable for tracking artist availabilty, write to ratedlib.dsv
        addIntervalValues();
        std::cout << "addIntervalValues completed." << std::endl;
        std::fstream filestr3;
        filestr3.open ("ratedlib.dsv");
        if (filestr3.is_open()) {
            filestr3.close();
            std::cout << "File ratedlib.dsv successfully created. Deleting rated.dsv." << std::endl;
            remove("rated.dsv");}
        else {std::cout << "Error opening ratedlib.dsv.dsv file before deleting rated.dsv" << std::endl;}

    //Section beyond initial program setup
    // Run function getArtistExcludes to populate a list of artists currently unavailable
    getArtistExcludes();
    std::cout << "getArtistExcludes completed." << std::endl;
    // Run function getArtistExcludes2 to populate a list of artists currently unavailable
    getArtistExcludes2();
    std::cout << "getArtistExcludes2 completed." << std::endl;
    std::cout << "Current playlist size is: "<< s_playlistSize << std::endl;
    //unsigned long histCount = s_SequentialTrackLimit - s_playlistSize;
    static long s_histCount = long(s_SequentialTrackLimit) - long(s_playlistSize);
    std::cout << "History count for calculating additional artist excludes: "<< s_histCount << std::endl;
    getExtendedExcludes(&s_histCount,&s_playlistSize);
    std::cout << "getExtendedExcludes completed." << std::endl;
    appendExcludes();
    std::cout << "appendExcludes completed." << std::endl;
    fixsort();
    int ratingNextTrack;
    ratingNextTrack = ratingCodeSelected(&s_ratingRatio3,&s_ratingRatio4,&s_ratingRatio5,&s_ratingRatio6,&s_ratingRatio7,&s_ratingRatio8);
    std::cout << "Rating for the next track is " << ratingNextTrack << std::endl;
    std::cout << "done!" << std::endl;
}
