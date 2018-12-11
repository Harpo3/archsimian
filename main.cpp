#include <QApplication>
#include <QFileInfo>
#include <iostream>
#include <fstream>
#include <string>
#include <QDir>
#include <sstream>
#include <ctime>
#include <chrono>
#include <cassert>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sqlite3.h>
#include <unistd.h>
#include <stdio.h>
#include "archsimian.h"
#include "dependents.h"
#include "userconfig.h"
#include "runbashapp.h"
#include "lastplayeddays.h"
#include "database.h"
#include "constants.h"

using namespace userconfig;
void load_config_file(int x);
int isConfigSetup(int x);

//Function to execute command line with parameters
template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N]) {
  assert((N > 0) && (argv[N - 1] == nullptr));
  return execvp(file, const_cast<char* const*>(argv));
}

int main()
{

    //Basic info: argc (argument count) and argv (argument vector) are
    //how command line arguments are passed to main() in C++.
    //argc will be the number of strings pointed to by argv. This will
    //(in practice) be 1 plus the number of arguments, as virtually all
    //implementations will prepend the name of the program to the array.

    // User configuration: call the isConfigSetup function to set a const bool used for prompting user
    // to setup the program settings before program operation.

    pid_t c_pid;
    c_pid = fork(); //duplicate

    // Get songs table from MM4 database, and save as libtable.dsv;
    // This fork is to write libtable.dsv first before further processing
    if( c_pid == 0 ){
        std::cout << "Child pid # (fork to write libtable.dsv): " << c_pid << std::endl;
        std::string mmbackupdbdirname = getConfigEntry(5); // 1=music lib, 3=playlist, 5=mm.db dir
        const std::string sqlpathdirname = "/home/lpc123"; // revise for QStandardPaths class if this does
                                                       //not set with makefile for this location
        std::string path1 = mmbackupdbdirname + "/MM.DB";
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
    else if (c_pid > 0){
       // Gather stats;
       // First, output library with cleaned track paths to cleanlib.dsv
       sleep(1);  // delay to provide time for child process to write libtable.dsv
       std::string databaseFile = "libtable.dsv"; // now use it as input file
       std::ofstream outf("cleanlib.dsv"); // and write to this after cleaned up
       std::ifstream data(databaseFile);
       std::fstream filestr1;
       filestr1.open ("libtable.dsv");
         if (filestr1.is_open()) {filestr1.close();}
         else {std::cout << "Error opening libtable.dsv file after it was created in child process." << std::endl;}

       // Declare statistical variables to be collected

       // First group is to get times and track quantities by rating group
       int rCode0TotTrackQty{0};
       int rCode0MsTotTime{0};
       int rCode1TotTrackQty{0};
       int rCode1MsTotTime{0};
       int rCode3TotTrackQty{0};
       int rCode3MsTotTime{0};
       int rCode4TotTrackQty{0};
       int rCode4MsTotTime{0};
       int rCode5TotTrackQty{0};
       int rCode5MsTotTime{0};
       int rCode6TotTrackQty{0};
       int rCode6MsTotTime{0};
       int rCode7TotTrackQty{0};
       int rCode7MsTotTime{0};
       int rCode8TotTrackQty{0};
       int rCode8MsTotTime{0};

       // Second group is to get recently played statistics for last six 10-day periods

       // First range is between yesterday and eleven days ago, so first count is
       // all values between (currSQLDate - 1) and (currSQLDate - 10.999)
       // then (currSQLDate - 11) and (currSQLDate - 20.999) and so on to 60.999
       // so anything greater than currSQLDate - 60.999 is counted, then subcounted to each 10 day period
       int SQL10TotTimeListened{0};
       int SQL10DayTracksTot{0};
       double SQL20TotTimeListened{0};
       int SQL20DayTracksTot{0};
       double SQL30TotTimeListened{0};
       int SQL30DayTracksTot{0};
       double SQL40TotTimeListened{0};
       int SQL40DayTracksTot{0};
       double SQL50TotTimeListened{0};
       int SQL50DayTracksTot{0};
       double SQL60TotTimeListened{0};
       int SQL60DayTracksTot{0};

       std::string musiclibrarydirname = getConfigEntry(1); // 1=music lib, 3=playlist, 5=mm.db dir

       double currDate = std::chrono::duration_cast<std::chrono::seconds>
               (std::chrono::system_clock::now().time_since_epoch()).count(); // This will go to lastplayed .cpp and .h
       // The conversion formula for epoch time to SQL time is: x = (x / 86400) + 25569  43441.4712847 43440.4712847
       double currSQLDate = (currDate / 86400) + 25569;        // This will go to lastplayed .cpp and .h

       if (!data.is_open())
           {
               std::exit(EXIT_FAILURE);
           }
       std::string str;
       int stringCount{0};
       while (std::getline(data, str))
           {
               std::istringstream iss(str);
               std::string strnew;
               std::string token;
               std::string tempTokenTrackTime;
               std::string str2 ("\\");               
               int tokenCount{0};

               while (std::getline(iss, token, '^')) {

                   // TOKEN PROCESSING - COL 8
                   // Col 8 is the song path, which needs to be corrected for linux and library user path specified
                   if (tokenCount == 8) { // Fix the dir string: remove colon, insert library user path,
                       // and correct the dir symbol from \ to /
                       std::size_t found = str.find(str2);
                       if (found!=std::string::npos)
                       {// colon is one char before the first dir symbol
                           str.replace(str.find(str2)-1,str2.length()+1,musiclibrarydirname + "/");
                           found=str.find("second dir symbol",found+1,1);
                           str.replace(str.find(str2),str2.length(),"/");
                           found=str.find("third dir symbol",found+1,1);
                           str.replace(str.find(str2),str2.length(),"/");
                       }
                       token = str;
                   }
                   // Store the time in milliseconds (col 12) in tempTokenTrackTime variable
                   if (tokenCount == 12) {tempTokenTrackTime = token;}

                   // TOKEN PROCESSING - COL 17
                   // Col 17 is used to collect lastplayed stats to compute avg listening time
                   double tempLastPlayedDate{0};
                   if (tokenCount == 17) {
                       tempLastPlayedDate = ::atof(token.c_str());//valid C++ syntax to convert a token string to a double.
                       if (tempLastPlayedDate > (currSQLDate - 60.9999))
                       {
                           if (tempLastPlayedDate > (currSQLDate - 10.9999))
                           {int timeint = stoi(tempTokenTrackTime);
                               SQL10TotTimeListened = SQL10TotTimeListened + timeint;
                               ++SQL10DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 11) && (tempLastPlayedDate > (currSQLDate - 20.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               SQL20TotTimeListened = SQL20TotTimeListened + timeint;
                               ++SQL20DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 21) && (tempLastPlayedDate > (currSQLDate - 30.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               SQL30TotTimeListened = SQL30TotTimeListened + timeint                                ;
                               ++SQL30DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 31) && (tempLastPlayedDate > (currSQLDate - 40.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               SQL40TotTimeListened = SQL40TotTimeListened + timeint;
                               ++SQL40DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 41) && (tempLastPlayedDate > (currSQLDate - 50.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               SQL50TotTimeListened = SQL50TotTimeListened + timeint;
                               ++SQL50DayTracksTot;}
                           if (tempLastPlayedDate <= (currSQLDate - 51) && (tempLastPlayedDate > (currSQLDate - 60.9999)))
                               {int timeint = stoi(tempTokenTrackTime);
                               SQL60TotTimeListened = SQL60TotTimeListened + timeint;
                               ++SQL60DayTracksTot;}
                           }
                       }
                   // TOKEN PROCESSING - COLS 12, 29
                   // Then, check the current line for the GroupDesc (rating code, col 29), which is
                   // then used to increment the song quantity and song time accordingly
                   // Cols 12 and 29 are for song time and GroupDesc (rating code)
                   // Collect statistical data for times and qtys of each rating category
                       if (tokenCount == 29 && token == "0") {
                           int timeint = stoi(tempTokenTrackTime);
                           rCode0MsTotTime = rCode0MsTotTime + timeint;
                           ++rCode0TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "1") {
                           int timeint = stoi(tempTokenTrackTime);
                           rCode1MsTotTime = rCode1MsTotTime + timeint;
                           ++rCode1TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "3") {
                           int timeint = stoi(tempTokenTrackTime);
                           rCode3MsTotTime = rCode3MsTotTime + timeint;
                           ++rCode3TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "4") {
                           int timeint = stoi(tempTokenTrackTime);
                           rCode4MsTotTime = rCode4MsTotTime + timeint;
                           ++rCode4TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "5") {
                           int timeint = stoi(tempTokenTrackTime);
                           rCode5MsTotTime = rCode5MsTotTime + timeint;
                           ++rCode5TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "6") {
                           int timeint = stoi(tempTokenTrackTime);
                           rCode6MsTotTime = rCode6MsTotTime + timeint;
                           ++rCode6TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "7") {
                           int timeint = stoi(tempTokenTrackTime);
                           rCode7MsTotTime = rCode7MsTotTime + timeint;
                           ++rCode7TotTrackQty;
                       }
                       if (tokenCount == 29 && token == "8") {
                           int timeint = stoi(tempTokenTrackTime);
                           rCode8MsTotTime = rCode8MsTotTime + timeint;
                           ++rCode8TotTrackQty;
                       }
                       ++ tokenCount;
                   }
                   outf << str << std::endl; // The string is valid, write to clean file
                   ++ stringCount;
               }

           // Close files opened for reading and writing
           data.close();
           outf.close();

       // Compile statistics and print results to console for program integration tasks later
       int totalLibQty = rCode0TotTrackQty + rCode1TotTrackQty + rCode3TotTrackQty + rCode4TotTrackQty +
               rCode5TotTrackQty + rCode6TotTrackQty + rCode7TotTrackQty + rCode8TotTrackQty;
       int totalRatedQty = totalLibQty - rCode0TotTrackQty;

       std::cout << "Total tracks Rating 0: " << rCode0TotTrackQty << ". Total Time (hrs): " <<  (rCode0MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 1: " << rCode1TotTrackQty << ". Total Time (hrs): " <<  (rCode1MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 3: " << rCode3TotTrackQty << ". Total Time (hrs): " <<  (rCode3MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 4: " << rCode4TotTrackQty << ". Total Time (hrs): "<<  (rCode4MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 5: " << rCode5TotTrackQty << ". Total Time (hrs): "<< (rCode5MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 6: " << rCode6TotTrackQty << ". Total Time (hrs): "<<  (rCode6MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 7: " << rCode7TotTrackQty << ". Total Time (hrs): " <<  (rCode7MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks Rating 8: " << rCode8TotTrackQty << ". Total Time (hrs): " <<  (rCode8MsTotTime/60000)/60 << std::endl;
       std::cout << "Total tracks in the library is: " << totalLibQty << std::endl;
       std::cout << "Total rated tracks in the library is: " << totalRatedQty << std::endl;
       std::cout << "Total time listened for first 10-day period is (hrs): " << (SQL10TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the first period is: " << SQL10DayTracksTot << std::endl;
       std::cout << "Total time listened for second 10-day period is (hrs): " << (SQL20TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the second period is: " << SQL20DayTracksTot << std::endl;
       std::cout << "Total time listened for third 10-day period is (hrs): " << (SQL30TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the third period is: " << SQL30DayTracksTot << std::endl;
       std::cout << "Total time listened for fourth 10-day period is (hrs): " << (SQL40TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the fourth period is: " << SQL40DayTracksTot << std::endl;
       std::cout << "Total time listened for fifth 10-day period is (hrs): " << (SQL50TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the fifth period is: " << SQL50DayTracksTot << std::endl;
       std::cout << "Total time listened for sixth 10-day period is (hrs): " << (SQL60TotTimeListened/60000)/60 << std::endl;
       std::cout << "Total tracks played in the sixth period is: " << SQL60DayTracksTot << std::endl;
       //std::cout << "Estimated listening time per day is: "<< median (SQL10TotTimeListened,SQL20TotTimeListened, SQL30TotTimeListened,SQL40TotTimeListened, SQL50TotTimeListened, SQL60TotTimeListened) << std::endl;

       std::fstream filestr;
         filestr.open ("cleanlib.dsv");
         if (filestr.is_open())
         {
           std::cout << "File cleanlib.dsv successfully created. Deleting libtable.dsv." << std::endl;
           remove("libtable.dsv");
           filestr.close();
         }
         else
         {
           std::cout << "Error opening file" << std::endl;
         }

   printf("Parent process (to write cleanlib.dsv) pid #: %d\n", c_pid);
   }
    else {
     //error: The return of fork() is negative
     perror("fork failed");
     _exit(2); //exit failure, hard
   }
     return 0;

    //Basic info: Identifies the main GUI app (QApplication) as mainapp
    //QApplication mainapp(argc, argv);

    // This launches the user interface (UI)
    //ArchSimian guiWindow;
    //guiWindow.show();

    // This closes the files opened in the sample section above

            //mainapp.exec();
}







// moved (old):
//        std::cout << "\nNow let's test whether the variable values persist outside the function.\n";
//        std::cout << "musiclibrarydirname: " << musiclibrarydirname.toStdString() << ".\n";
//        std::cout << "mmbackuppldirname: " << mmbackuppldirname.toStdString() << ".\n";
//        std::cout << "mmbackupdbdirname: " << mmbackupdbdirname.toStdString() << ".\n";

// moved (old):
//QString appendPath(const QString& path1, const QString& path2)
//{
//    return QDir::cleanPath(path1 + QDir::separator() + path2);
//}
// This function determines the user's HOME path, which will be used to
// create and access configuration files

//moved old:
//   ofstream my_output_file;
//   localConfigFilePath.open(localConfigFilePath);
 //  my_output_file.open(localConfigFilePath);
//   if (my_input_file.fail() )
//     {
   //       // I need to recreate the file
//       std::cout << "The file is not there. Need to create one.";
 //     }
//      my_input_file.close();
//      my_output_file.close();

//   if (my_output_file.is_open())
//      {
//    my_output_file << "A test.";
//    my_output_file.close();
//
 //     }
//   return myConfig;
//   }

//moved (old):
//std::ifstream my_output_file;
//my_output_file.open(fullString);
//std::ostream >> "Testing 123";
//bool isopening;
//std::ofstream ifs(fullString);
//ifs.open(fullString);
//fs << "writing this to a file.\n";
//ifs.close();

// char* result; result = calloc(strlen(myStringPath)+strlen(two)+1, sizeof(char)); and THEN the strcpy+strcat?
 //  char result[75];
//   char* (strlen(char* myStringPath) + strlen(char* userFileName[17])+1, sizeof(char));

//   strcpy(result, userFileName);
  // std::cout << "\nThis is the first placeholder in main for some sample code to concatenate the file path,"
//                "\n with the filename. Output is a string type \nvariable and its result is: "
//             << result << ".\n\n";
//  strcat(result, myStringPath);
 //  std::cout << "\nThis is the second placeholder in main for some sample code to concatenate the file path,"
//                "\n with the filename. Output is a string type \nvariable and its result is: "
 //            << result << ".\n\n";

