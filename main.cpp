//#include <QApplication>
//#include <QFileInfo>
//#include <QDir>
//#include <cassert>
//#include <stdio.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <sys/wait.h>
//#include <sqlite3.h>
//#include <stdlib.h>
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <ctime>
//#include <chrono>
//#include <errno.h>
//#include <stdio.h>
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

    static int s_isConfigSetResult(userconfig::isConfigSetup());// Call the isConfigSetup function to check if config has been set up
    //std::cout << "The configuration is ";
    if (s_isConfigSetResult == 0) { // Evaluate and run gui for user config setup, if result is 0, otherwise continue
        std::cout << "The configuration is not set up: " << s_isConfigSetResult << ". Starting configuration setup in gui." << std::endl;
        QApplication mainapp(argc, argv);
        ArchSimian guiWindow;
        guiWindow.show();
        mainapp.exec();// launch gui
        }    
    pid_t c_pid;// Create fork object; parent to get database table, child to use table to clean it up
    c_pid = fork(); // Run fork function

    if( c_pid == 0 ){ // Parent process: Get songs table from MM4 database, and save as libtable.dsv;
        //std::cout << "Child pid (fork to write libtable.dsv), pid#: " << c_pid << std::endl;
        std::string s_mmbackupdbdirname = userconfig::getConfigEntry(5); // 1=music lib, 3=playlist, 5=mm.db dir
        // revise for QStandardPaths class if this does not set with makefile for this location
        const std::string sqlpathdirname = "/home/lpc123";
        std::string path1 = s_mmbackupdbdirname + "/MM.DB";
        //std::cout << "path1 is: " << path1 << ".\n";
        std::string path2 = ".read " + sqlpathdirname + "/exportMMTable.sql";
        //std::cout << "path2 is: " << path2 << ".\n";
        const char* const argv[] = {" ", path1.c_str(), path2.c_str(), nullptr};
        //std::cout << "Setting up path1: " << path1 << " and path 2: " << path2 << std::endl;
        execvp("sqlite3", argv);
        perror("execvp");
        if (execvp("sqlite3", argv) == -1)
            exit(EXIT_FAILURE);        
        }

    else if (c_pid > 0){  // Child process starts here. Write libtable.dsv and gather stats
       // First, reopen libtable.dsv, clean track paths, and output to cleanlib.dsv
       sleep(1);  // needs delay for child process to finish writing libtable.dsv
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

       // Compile statistics and declare additional statistical variables
       static int s_totalLibQty = s_rCode0TotTrackQty + s_rCode1TotTrackQty + s_rCode3TotTrackQty + s_rCode4TotTrackQty +
               s_rCode5TotTrackQty + s_rCode6TotTrackQty + s_rCode7TotTrackQty + s_rCode8TotTrackQty;
       static int s_totalRatedQty = s_totalLibQty - s_rCode0TotTrackQty;

       //Print results to console (for later program integration tasks (TBD)
       std::cout << "Total tracks Rating 0: " << s_rCode0TotTrackQty << ". Total Time (hrs): " <<  (s_rCode0MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 1: " << s_rCode1TotTrackQty << ". Total Time (hrs): " <<  (s_rCode1MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 3: " << s_rCode3TotTrackQty << ". Total Time (hrs): " <<  (s_rCode3MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 4: " << s_rCode4TotTrackQty << ". Total Time (hrs): "<<  (s_rCode4MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 5: " << s_rCode5TotTrackQty << ". Total Time (hrs): "<< (s_rCode5MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 6: " << s_rCode6TotTrackQty << ". Total Time (hrs): "<<  (s_rCode6MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 7: " << s_rCode7TotTrackQty << ". Total Time (hrs): " <<  (s_rCode7MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 8: " << s_rCode8TotTrackQty << ". Total Time (hrs): " <<  (s_rCode8MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks in the library is: " << s_totalLibQty << std::endl;
       std::cout << "Total rated tracks in the library is: " << s_totalRatedQty << std::endl;
       std::cout << "Total time listened for first 10-day period is (hrs): " << (s_SQL10TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the first period is: " << s_SQL10DayTracksTot << std::endl;
       std::cout << "Total time listened for second 10-day period is (hrs): " << (s_SQL20TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the second period is: " << s_SQL20DayTracksTot << std::endl;
       std::cout << "Total time listened for third 10-day period is (hrs): " << (s_SQL30TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the third period is: " << s_SQL30DayTracksTot << std::endl;
       std::cout << "Total time listened for fourth 10-day period is (hrs): " << (s_SQL40TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the fourth period is: " << s_SQL40DayTracksTot << std::endl;
       std::cout << "Total time listened for fifth 10-day period is (hrs): " << (s_SQL50TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the fifth period is: " << s_SQL50DayTracksTot << std::endl;
       std::cout << "Total time listened for sixth 10-day period is (hrs): " << (s_SQL60TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the sixth period is: " << s_SQL60DayTracksTot << std::endl;
       //std::cout << "Estimated listening time per day is: "<< median (s_SQL10TotTimeListened,s_SQL20TotTimeListened,s_SQL30TotTimeListened,s_SQL40TotTimeListened, SQL50TotTimeListened, SQL60TotTimeListened) << std::endl;

       std::fstream filestr;
         filestr.open ("cleanlib.dsv");
         if (filestr.is_open())
         {
           //std::cout << "File cleanlib.dsv successfully created. Deleting libtable.dsv." << std::endl;
           //remove("libtable.dsv");
           filestr.close();
         }
         else
         {
           std::cout << "Error opening file" << std::endl;
         }
         }
    else { // if (c_pid < 0) error check: The return of fork() is negative
        perror("fork failed");
        _exit(2); //exit failure, hard
        }

    getRatedTable();

    if (s_isConfigSetResult == 1) { //If user config setup was already run (result is 1) run GUI app here
                                    // which after the sts and file data have been loaded
    QApplication mainapp(argc, argv);
    ArchSimian guiWindow;
    guiWindow.show();// This launches the user interface (UI)
    mainapp.exec();
}
  return 0;
}
