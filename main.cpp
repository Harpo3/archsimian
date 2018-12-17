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
    std::cout << "The configuration is ";
    if (s_isConfigSetResult == 0) { // Evaluate and run gui for user config setup, if result is 0, otherwise continue
        std::cout << "Not set up: " << s_isConfigSetResult << ". Starting configuration setup in gui." << std::endl;
        QApplication mainapp(argc, argv);
        ArchSimian guiWindow;
        guiWindow.show();
        mainapp.exec();// launch gui
        }    
    pid_t c_pid;// Create fork object; parent to get database table, child to use table to clean it up
    c_pid = fork(); // Run fork function
    if( c_pid == 0 ){ // Parent process: Get songs table from MM4 database, and save as libtable.dsv;
        std::cout << "Child pid # (fork to write libtable.dsv): " << c_pid << std::endl;
        std::string s_mmbackupdbdirname = userconfig::getConfigEntry(5); // 1=music lib, 3=playlist, 5=mm.db dir
        // revise for QStandardPaths class if this does not set with makefile for this location
        const std::string sqlpathdirname = "/home/lpc123";
        std::string path1 = s_mmbackupdbdirname + "/MM.DB";
         std::cout << "path1 is: " << path1 << ".\n";
        std::string path2 = ".read " + sqlpathdirname + "/exportMMTable.sql";
        std::cout << "path2 is: " << path2 << ".\n";
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
       std::string databaseFile = "libtable.dsv"; // now we can use it as input file
       std::ofstream outf("cleanlib.dsv"); // output file for writing clean track paths
       std::ifstream primarySongsTable(databaseFile);


       // Declare statistical variables to be collected

       // First group is to get times and track quantities by rating group
       static int s_rCode0TotTrackQty{0};
       static int s_rCode0MsTotTime{0};
       static int s_rCode1TotTrackQty{0};
       static int s_rCode1MsTotTime{0};
       static int s_rCode3TotTrackQty{0};
       static int s_rCode3MsTotTime{0};
       static int s_rCode4TotTrackQty{0};
       static int s_rCode4MsTotTime{0};
       static int s_rCode5TotTrackQty{0};
       static int s_rCode5MsTotTime{0};
       static int s_rCode6TotTrackQty{0};
       static int s_rCode6MsTotTime{0};
       static int s_rCode7TotTrackQty{0};
       static int s_rCode7MsTotTime{0};
       static int s_rCode8TotTrackQty{0};
       static int s_rCode8MsTotTime{0};

       // Second group is to get recently played statistics for last six 10-day periods

       // First range is between yesterday and eleven days ago, so first count is
       // all values between (currSQLDate - 1) and (currSQLDate - 10.999)
       // then (currSQLDate - 11) and (currSQLDate - 20.999) and so on to 60.999
       // so anything greater than currSQLDate - 60.999 is counted, then subcounted to each 10 day period
       static int s_SQL10TotTimeListened{0};
       static int s_SQL10DayTracksTot{0};
       static double s_SQL20TotTimeListened{0};
       static int s_SQL20DayTracksTot{0};
       static double s_SQL30TotTimeListened{0};
       static int s_SQL30DayTracksTot{0};
       static double s_SQL40TotTimeListened{0};
       static int s_SQL40DayTracksTot{0};
       static double s_SQL50TotTimeListened{0};
       static int s_SQL50DayTracksTot{0};
       static double s_SQL60TotTimeListened{0};
       static int s_SQL60DayTracksTot{0};

       std::string s_musiclibrarydirname = userconfig::getConfigEntry(1); // 1=music lib, 3=playlist, 5=mm.db dir
       double currDate = std::chrono::duration_cast<std::chrono::seconds>
               (std::chrono::system_clock::now().time_since_epoch()).count(); // This will go to lastplayed .cpp and .h
       // The conversion formula for epoch time to SQL time is: x = (x / 86400) + 25569  43441.4712847 43440.4712847
       double currSQLDate = (currDate / 86400) + 25569;        // This will go to lastplayed .cpp and .h

       if (!primarySongsTable.is_open())
           {
               std::exit(EXIT_FAILURE);
           }
       std::string str;
       int stringCount{0};
       while (std::getline(primarySongsTable, str)) {   // Outer loop: iterate through rows of primary songs table
               // Declare variables applicable to all rows
               std::istringstream iss(str);
               std::string strnew;
               std::string token;
               std::string tempTokenTrackTime;
               std::string tempTokenLastPlayedTime{"0.0"};
               int tokenCount{0};

               while (std::getline(iss, token, '^')) { // Inner loop: iterate through each column (token) of row
                   // TOKEN PROCESSING - COL 8
                   // Col 8 is the song path, which needs to be corrected for linux and library user path specified
                   if (tokenCount == 8) { // Fix the dir string: remove colon, insert library user path,
                       // and correct the dir symbol from \ to /
                       std::string str2 ("\\");
                       std::size_t found = str.find(str2);
                       if (found!=std::string::npos)
                       {// colon is one char before the first dir symbol
                           str.replace(str.find(str2)-1,str2.length()+1,s_musiclibrarydirname + "/");
                           found=str.find("second dir symbol",found+1,1);
                           str.replace(str.find(str2),str2.length(),"/");
                           found=str.find("third dir symbol",found+1,1);
                           str.replace(str.find(str2),str2.length(),"/");
                       }
                       token = str; // set token to revised string with corrected path
                   }
                   // Store the time in milliseconds (col 12) in tempTokenTrackTime text variable
                   if (tokenCount == 12) {tempTokenTrackTime = token;}
                   // Store the lastplayed date string tempTokenLastPlayedTime text variable
                   if (tokenCount == 17) {tempTokenLastPlayedTime = token;}

                   // TOKEN PROCESSING - COL 17
                   // Collect lastplayed stats to compute avg listening time
                   double tempLastPlayedDate{0};
                   std::string strrandom;
                   if (tokenCount == 17) {
                       tempLastPlayedDate = ::atof(token.c_str()); //valid C++ syntax to convert a token string to a double.
                       if (tempLastPlayedDate > (currSQLDate - 60.9999))
                       {
                           if (tempLastPlayedDate > (currSQLDate - 10.9999))
                           {int timeint = stoi(tempTokenTrackTime);
                               s_SQL10TotTimeListened = s_SQL10TotTimeListened + timeint;
                               ++s_SQL10DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 11) && (tempLastPlayedDate > (currSQLDate - 20.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               s_SQL20TotTimeListened = s_SQL20TotTimeListened + timeint;
                               ++s_SQL20DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 21) && (tempLastPlayedDate > (currSQLDate - 30.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               s_SQL30TotTimeListened = s_SQL30TotTimeListened + timeint                                ;
                               ++s_SQL30DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 31) && (tempLastPlayedDate > (currSQLDate - 40.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               s_SQL40TotTimeListened = s_SQL40TotTimeListened + timeint;
                               ++s_SQL40DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 41) && (tempLastPlayedDate > (currSQLDate - 50.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               s_SQL50TotTimeListened = s_SQL50TotTimeListened + timeint;
                               ++s_SQL50DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 51) && (tempLastPlayedDate > (currSQLDate - 60.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               s_SQL60TotTimeListened = s_SQL60TotTimeListened + timeint;
                               ++s_SQL60DayTracksTot;}
                           }
                   }
                   // TOKEN PROCESSING - COLS 12, 29
                   // Then, check the current line for the GroupDesc (rating code, col 29), which is
                   // then used to increment the song quantity and song time accordingly
                   // Cols 12 and 29 are for song time and GroupDesc (rating code)
                   // Collect statistical data for times and qtys of each rating category
                   if (tokenCount == 29 && token == "0") {
                           int timeint = stoi(tempTokenTrackTime);
                           s_rCode0MsTotTime = s_rCode0MsTotTime + timeint;
                           ++s_rCode0TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "1") {
                           int timeint = stoi(tempTokenTrackTime);
                           s_rCode1MsTotTime = s_rCode1MsTotTime + timeint;
                           ++s_rCode1TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "3") {
                           int timeint = stoi(tempTokenTrackTime);
                           s_rCode3MsTotTime = s_rCode3MsTotTime + timeint;
                           ++s_rCode3TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "4") {
                           int timeint = stoi(tempTokenTrackTime);
                           s_rCode4MsTotTime = s_rCode4MsTotTime + timeint;
                           ++s_rCode4TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "5") {
                           int timeint = stoi(tempTokenTrackTime);
                           s_rCode5MsTotTime = s_rCode5MsTotTime + timeint;
                           ++s_rCode5TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "6") {
                           int timeint = stoi(tempTokenTrackTime);
                           s_rCode6MsTotTime = s_rCode6MsTotTime + timeint;
                           ++s_rCode6TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "7") {
                           int timeint = stoi(tempTokenTrackTime);
                           s_rCode7MsTotTime = s_rCode7MsTotTime + timeint;
                           ++s_rCode7TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "8") {
                           int timeint = stoi(tempTokenTrackTime);
                           s_rCode8MsTotTime = s_rCode8MsTotTime + timeint;
                           ++s_rCode8TotTrackQty;
                       }
                       ++ tokenCount;
                   }
                   outf << str << std::endl; // The string is valid, write to clean file
                   ++ stringCount;
               }
           // Close files opened for reading and writing
           primarySongsTable.close();
           outf.close();

       // Compile statistics and print results to console for program integration tasks (TBD)
       int totalLibQty = s_rCode0TotTrackQty + s_rCode1TotTrackQty + s_rCode3TotTrackQty + s_rCode4TotTrackQty +
               s_rCode5TotTrackQty + s_rCode6TotTrackQty + s_rCode7TotTrackQty + s_rCode8TotTrackQty;
       int totalRatedQty = totalLibQty - s_rCode0TotTrackQty;

       std::cout << "Total tracks Rating 0: " << s_rCode0TotTrackQty << ". Total Time (hrs): " <<  (s_rCode0MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 1: " << s_rCode1TotTrackQty << ". Total Time (hrs): " <<  (s_rCode1MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 3: " << s_rCode3TotTrackQty << ". Total Time (hrs): " <<  (s_rCode3MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 4: " << s_rCode4TotTrackQty << ". Total Time (hrs): "<<  (s_rCode4MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 5: " << s_rCode5TotTrackQty << ". Total Time (hrs): "<< (s_rCode5MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 6: " << s_rCode6TotTrackQty << ". Total Time (hrs): "<<  (s_rCode6MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 7: " << s_rCode7TotTrackQty << ". Total Time (hrs): " <<  (s_rCode7MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 8: " << s_rCode8TotTrackQty << ". Total Time (hrs): " <<  (s_rCode8MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks in the library is: " << totalLibQty << std::endl;
       std::cout << "Total rated tracks in the library is: " << totalRatedQty << std::endl;
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

    //////////////////////

    std::fstream filestr2;
    filestr2.open ("cleanlib.dsv");
    if (filestr2.is_open()) {filestr2.close();}
    else {std::cout << "Error opening cleanlib.dsv file after it was created in child process." << std::endl;}
    std::string cleanlibrary = "cleanlib.dsv"; // now we can use it as input file
    std::ofstream outfrated("rated.dsv"); // output file for writing clean track paths
    std::ifstream cleanedSongsTable(cleanlibrary);
    bool skiprow{false};

    // Check rating (stars) code (col 13), then lastplayed (col 17), then GroupDesc (col 29)
    // Evaluate if col 17 is "0.0" if so, replace string with random lastplayed date. Then compare
    // col 13 data to col 29 val, and if 29 is not one of the ratings codes, use 13 to assign the
    // correct code. Later version: when user opts to write to tags, write changed TIT1 value to tag

    if (!cleanedSongsTable.is_open())
    {
        std::cout << "Error opening cleanedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str;
    int stringCount{0};
    while (std::getline(cleanedSongsTable, str)) {   // Outer loop: iterate through rows of cleanedSongsTable
        // Declare variables applicable to all rows
        std::istringstream iss(str);
        std::string strnew;
        std::string token;
        std::string tempTokenStarRating; //used to filter rows where star rating is zero;
        int tokenCount{0}; //token count is the number of delimiter characters within the string
        std::string strrandom; // store random number generated in a text variable
        skiprow=false;

        while (std::getline(iss, token, '^'))
        {
            // Inner loop: iterate through each column (token) of row
            // TOKEN PROCESSING - COL 13 (delimiter # 12)
            // Store the star rating (col 13) in the tempTokenStarRating text variable
            if (tokenCount == 13) {tempTokenStarRating = token;}

            // TOKEN PROCESSING - COL 17 (delimiter #15 -text- delimiter #16)
            // Evaluates whether a lastplayed date (col 17) is zero, if so, replace with a random date
            //if (tokenCount == 17) std::cout << token << std::endl;
            if (tempTokenStarRating != "0" && tokenCount == 17 && token == ("0.0"))
                //&& tempTokenStarRating != "0")// generate a random lastplayed date if its current
                //  value is "0" unless track has a zero star rating
            {
                //std::cout << "Col 17 DateLastPlayed not found for: " << str << std::endl;
                // Process a function to generate a random date 30-500 days ago then save to a string
                double rndresult{0.0};
                int intconvert;
                rndresult = getNewRandomLPDate(rndresult);
                if (rndresult == 0.0)
                {std::cout << "Error obtaining random number at row: " << token << std::endl;}
                intconvert = int (rndresult); // convert the random number to an integer
                strrandom = std::to_string(intconvert); // convert the integer to string

                signed int poscount = 0;
                signed int myspot = 0;
                std::size_t found = str.find_first_of("^");
                while (found!=std::string::npos)
                {
                    if (poscount == 16) {myspot = found+1;}
                    found=str.find_first_of("^",++found);
                    ++poscount;
                }
                str.replace(myspot,3,strrandom);
            }
            // TOKEN PROCESSING - COLS 13, 29
            // Then, check the current line for the GroupDesc (rating code, col 29), which is
            // then compared with the col 13 star rating temp variable
            // If there is no rating and a "0" is assigned as a rating code,
            // do not write the row to new file
            if (tokenCount == 29 && token == "0") {
                // do not write row - set skip variable skiprow
                skiprow = true;
            }
            ++ tokenCount;
        }
        if (skiprow == false) { // If the track is rated
            outfrated << str << std::endl; // The string is valid, write to clean file
        }
    }
    ++ stringCount;

    // Close files opened for reading and writing
    cleanedSongsTable.close();
    outfrated.close();

    ///////////////////////////////

    if (s_isConfigSetResult == 1) { //If user config setup was already run (result is 1) run GUI app here
                                    // which after the sts and file data have been loaded
    QApplication mainapp(argc, argv);
    ArchSimian guiWindow;
    guiWindow.show();// This launches the user interface (UI)
    mainapp.exec();
}
  return 0;
}
