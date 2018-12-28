#include <sstream>
#include <unistd.h>
#include "archsimian.h"
#include "dependents.h"
#include "userconfig.h"
#include "runbashapp.h"
#include "lastplayeddays.h"
#include "constants.h"
#include "getrated.h"
#include "getcleanlib.h"
#include "getplaylist.h"


template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N]) {//Function to execute command line with parameters
  assert((N > 0) && (argv[N - 1] == nullptr));
  return execvp(file, const_cast<char* const*>(argv));
}

int main(int argc,char* argv[])
{
    static std::string s_mmbackupdbdirname;
    static std::string s_musiclibrarydirname;
    static std::string s_mmbackuppldirname;
    static std::string s_selectedplaylist;
     // Set variable for customArtistID, either dir tree (0) or custom groupings (1)
    static bool customArtistID{false};
    static int s_isConfigSetResult(userconfig::isConfigSetup());// Call the isConfigSetup function to check if config has been set up
    if (s_isConfigSetResult == 0) { // Evaluate and run gui for user config setup, if result is 0, otherwise continue
        std::cout << "The configuration is not set up: " << s_isConfigSetResult << ". Starting configuration setup in gui." << std::endl;
        QApplication mainapp(argc, argv);
        ArchSimian guiWindow;
        guiWindow.show(); // This launches the user interface (UI) for configuration
        mainapp.exec();
        }
    //write function to execute from here to create and write a sql file to user's home directory
    pid_t c_pid;// Create fork object; parent to get database table, child to use table to clean it up
    c_pid = fork(); // Run fork function

    if( c_pid == 0 ){ // Parent process: Get songs table from MM4 database, and save as libtable.dsv;
        std::string s_mmbackupdbdirname = userconfig::getConfigEntry(5); // 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath        
        // revise for QStandardPaths class if this does not set with makefile for this location
        const std::string sqlpathdirname = getenv("HOME");
        std::string path1 = s_mmbackupdbdirname + "/MM.DB";
        std::string path2 = ".read " + sqlpathdirname + "/exportMMTable.sql";
        const char* const argv[] = {" ", path1.c_str(), path2.c_str(), nullptr};
        execvp("sqlite3", argv);
        perror("execvp");
        if (execvp("sqlite3", argv) == -1)
            exit(EXIT_FAILURE);        }
        // add function here to delete sql file from user's home directory after completion

    else if (c_pid > 0){  // Child process starts here. Write libtable.dsv and gather stats
       // First, reopen libtable.dsv, clean track paths, and output to cleanlib.dsv
       sleep(2);  // needs delay for child process to finish writing libtable.dsv
       std::fstream filestr1;
       filestr1.open ("libtable.dsv");
         if (filestr1.is_open()) {filestr1.close();}
         else {std::cout << "Error opening libtable.dsv file after it was created in child process." << std::endl;}

         // Declare statistical variables to be collected
         // First group is to get times and track quantities by rating group
         static int s_rCode0TotTrackQty{0}, s_rCode0MsTotTime{0},
         s_rCode1TotTrackQty{0}, s_rCode1MsTotTime{0},s_rCode3TotTrackQty{0},s_rCode3MsTotTime{0},
         s_rCode4TotTrackQty{0}, s_rCode4MsTotTime{0},s_rCode5TotTrackQty{0},s_rCode5MsTotTime{0},
         s_rCode6TotTrackQty{0},s_rCode6MsTotTime{0},s_rCode7TotTrackQty{0},s_rCode7MsTotTime{0},
         s_rCode8TotTrackQty{0},s_rCode8MsTotTime{0};

         // Second group is to get recently played statistics for last six 10-day periods
         // First range is between yesterday and eleven days ago, so first count is
         // all values between (currSQLDate - 1) and (currSQLDate - 10.999)
         // then (currSQLDate - 11) and (currSQLDate - 20.999) and so on to 60.999
         // so anything greater than currSQLDate - 60.999 is counted, then subcounted to each 10 day period
         static int s_SQL10TotTimeListened{0},s_SQL10DayTracksTot{0},s_SQL20DayTracksTot{0},
         s_SQL30DayTracksTot{0},s_SQL40DayTracksTot{0},s_SQL50DayTracksTot{0},s_SQL60DayTracksTot{0};
         static double s_SQL20TotTimeListened{0},s_SQL30TotTimeListened{0},s_SQL40TotTimeListened{0},
         s_SQL50TotTimeListened{0},s_SQL60TotTimeListened{0};

        // Function to fix dir path for linux and obtain the values for statistical variables, creates cleanlib.dsv
         getCleanLib(&s_rCode0TotTrackQty,&s_rCode0MsTotTime,&s_rCode1TotTrackQty,&s_rCode1MsTotTime,&s_rCode3TotTrackQty,&s_rCode3MsTotTime,
                     &s_rCode4TotTrackQty,&s_rCode4MsTotTime, &s_rCode5TotTrackQty,&s_rCode5MsTotTime,&s_rCode6TotTrackQty,
                     &s_rCode6MsTotTime, &s_rCode7TotTrackQty, &s_rCode7MsTotTime,&s_rCode8TotTrackQty, &s_rCode8MsTotTime,
                     &s_SQL10TotTimeListened, &s_SQL10DayTracksTot, &s_SQL20TotTimeListened,&s_SQL20DayTracksTot, &s_SQL30TotTimeListened,
                     &s_SQL30DayTracksTot,&s_SQL40TotTimeListened, &s_SQL40DayTracksTot, &s_SQL50TotTimeListened,
                     &s_SQL50DayTracksTot, &s_SQL60TotTimeListened, &s_SQL60DayTracksTot);

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
       static int s_totalLibQty = s_rCode0TotTrackQty + s_rCode1TotTrackQty + s_rCode3TotTrackQty + s_rCode4TotTrackQty +
               s_rCode5TotTrackQty + s_rCode6TotTrackQty + s_rCode7TotTrackQty + s_rCode8TotTrackQty;
        //Total number of rated tracks in the library
       static int s_totalRatedQty = s_totalLibQty - s_rCode0TotTrackQty;
        //Total listened hours in the last 60 days
       static double s_totHrsLast60Days = s_SQL10TotTimeListened + s_SQL20TotTimeListened + s_SQL30TotTimeListened + s_SQL40TotTimeListened
               + s_SQL50TotTimeListened + s_SQL60TotTimeListened;
        // User listening rate weighted avg calculated using the six 10-day periods, and applying sum-of-the-digits for weighting
       static double s_listeningRate = ((s_SQL10TotTimeListened/10)*0.3) + ((s_SQL20TotTimeListened/10)*0.25)  + ((s_SQL30TotTimeListened/10)*0.2) +
               ((s_SQL40TotTimeListened/10)*0.15) + ((s_SQL50TotTimeListened/10)*0.1) + ((s_SQL60TotTimeListened/10)*0.05);

       // Repeat factor codes used to calculate repeat rate in years
       static double s_daysTillRepeatCode3 = 75.0;
       static double s_yrsTillRepeatCode3 = s_daysTillRepeatCode3 / 365;
       static double s_repeatFactorCode4 = 2.7;
       static double s_yrsTillRepeatCode4 = s_yrsTillRepeatCode3 * s_repeatFactorCode4;
       static double s_repeatFactorCode5 = 2.1;
       static double s_yrsTillRepeatCode5 = s_yrsTillRepeatCode4 * s_repeatFactorCode5;
       static double s_repeatFactorCode6 = 2.3;
       static double s_yrsTillRepeatCode6 = s_yrsTillRepeatCode5 * s_repeatFactorCode6;
       static double s_repeatFactorCode7 = 1.8;
       static double s_yrsTillRepeatCode7 = s_yrsTillRepeatCode6 * s_repeatFactorCode7;
       static double s_repeatFactorCode8 = 1.2;
       static double s_yrsTillRepeatCode8 = s_yrsTillRepeatCode7 * s_repeatFactorCode8;
       static double s_adjHoursCode3 = (1 / s_yrsTillRepeatCode3) * s_rCode3TotTime;
       static double s_adjHoursCode4 = (1 / s_yrsTillRepeatCode4) * s_rCode4TotTime;
       static double s_adjHoursCode5 = (1 / s_yrsTillRepeatCode5) * s_rCode5TotTime;
       static double s_adjHoursCode6 = (1 / s_yrsTillRepeatCode6) * s_rCode6TotTime;
       static double s_adjHoursCode7 = (1 / s_yrsTillRepeatCode7) * s_rCode7TotTime;
       static double s_adjHoursCode8 = (1 / s_yrsTillRepeatCode8) * s_rCode8TotTime;
       static double s_totAdjHours = s_adjHoursCode3 + s_adjHoursCode4 + s_adjHoursCode5 + s_adjHoursCode6 +s_adjHoursCode7 + s_adjHoursCode8;
       static double s_ratingRatio3 = s_adjHoursCode3 / s_totAdjHours;
       static double s_ratingRatio4 = s_adjHoursCode4 / s_totAdjHours;
       static double s_ratingRatio5 = s_adjHoursCode5 / s_totAdjHours;
       static double s_ratingRatio6 = s_adjHoursCode6 / s_totAdjHours;
       static double s_ratingRatio7 = s_adjHoursCode7 / s_totAdjHours;
       static double s_ratingRatio8 = s_adjHoursCode8 / s_totAdjHours;

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

       std::cout << "Percentage of track time for scheduling rating code 3 - s_ratingRatio3 * 100 : "<< s_ratingRatio3 * 100 << "%" << std::endl;
       std::cout << "Percentage of track time for scheduling rating code 4 - s_ratingRatio4 * 100 : "<< s_ratingRatio4  * 100 << "%" << std::endl;
       std::cout << "Percentage of track time for scheduling rating code 5 - s_ratingRatio5 * 100 : "<< s_ratingRatio5  * 100 << "%" << std::endl;
       std::cout << "Percentage of track time for scheduling rating code 6 - s_ratingRatio6 * 100 : "<< s_ratingRatio6 * 100 <<  "%" << std::endl;
       std::cout << "Percentage of track time for scheduling rating code 7 - s_ratingRatio7 * 100 : "<< s_ratingRatio7 * 100 <<  "%" << std::endl;
       std::cout << "Percentage of track time for scheduling rating code 8 - s_ratingRatio8 * 100 : "<< s_ratingRatio8 * 100 <<  "%" << std::endl;

       std::fstream filestr;
       filestr.open ("cleanlib.dsv");
       if (filestr.is_open()) {filestr.close();}
           //std::cout << "File cleanlib.dsv successfully created. Deleting libtable.dsv." << std::endl;
           //remove("libtable.dsv");
       else {std::cout << "Error opening file" << std::endl;}
    }

    else { // if (c_pid < 0) error check: The return of fork() is negative
        perror("fork failed");
        _exit(2); //exit failure, hard
        }
    // Currently disabled, getReformattedTable is used to change the table to a comma-separated file for using in a class object.
    //getReformattedTable();


    // Using the function getPlaylist, remove formatting lines and correct path for playlist
    getPlaylist();

    // Using the function getPlaylistVect, load the current playlist into a vector plStrings

    std::vector<std::string> plStrings;
    getPlaylistVect("cleanedplaylist.txt", plStrings);

    // Using libtable.dsv from parent process create rated.dsv with random lastplayed dates created for
    // unplayed (but rated or new need-to-be-rated tracks with no play history)
    getRatedTable();

    // Not yet written: Using rated.dsv run function getadjartistratedcount to calculate the artist factors, write value to rated.dsv

    // Not yet written: Run function getartisttracksago to baseline the variable for tracking artist availabilty, write to rated.dsv

    // To set up artist-related data, determine the identifier for artists (add selector to GUI configuration)
    customArtistID = 1; // manually set to true (means use Custom 2 for artist)

    // Not yet written: Using the cleaned playlist, create a subset (available.dsv) of rated.dsv with playlist tracks removed

    // Section to launch GUI; uncomment last four lines below to enable
    if (s_isConfigSetResult == 1) { //If user config setup was already run (result is 1) run GUI app here
                                    // which after the sts and file data have been loaded
// GUI currently disabled
//    QApplication mainapp(argc, argv);
//    ArchSimian guiWindow;
   // guiWindow.show();   // This launches the user interface (UI)
    //mainapp.exec();
}
  return 0;
}
