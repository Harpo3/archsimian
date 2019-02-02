#include <sstream>
#include <unistd.h>
#include "userconfig.h"
#include "constants.h"

void getCleanLib(int *_srCode0TotTrackQty,int *_srCode0MsTotTime,int *_srCode1TotTrackQty,int *_srCode1MsTotTime,
                 int *_srCode3TotTrackQty,int *_srCode3MsTotTime,int *_srCode4TotTrackQty,int *_srCode4MsTotTime,
                 int *_srCode5TotTrackQty,int *_srCode5MsTotTime,int *_srCode6TotTrackQty,int *_srCode6MsTotTime,
                 int *_srCode7TotTrackQty,int *_srCode7MsTotTime,int *_srCode8TotTrackQty,int *_srCode8MsTotTime,
                 int *_sSQL10TotTimeListened,int *_sSQL10DayTracksTot,double *_sSQL20TotTimeListened,
                 int *_sSQL20DayTracksTot,double *_sSQL30TotTimeListened,int *_sSQL30DayTracksTot,double *_sSQL40TotTimeListened,
                 int *_sSQL40DayTracksTot,double *_sSQL50TotTimeListened,int *_sSQL50DayTracksTot,double *_sSQL60TotTimeListened,
                 int *_sSQL60DayTracksTot) {
    std::ifstream filestr1;
    filestr1.open ("libtable.dsv");
    if (filestr1.is_open()) {filestr1.close();}
    else {std::cout << "getCleanLib: Error opening libtable.dsv file just after it was created in parent process." << std::endl;}
    std::string databaseFile = "libtable.dsv"; // now we can use it as input file
    std::ofstream outf("cleanlib.dsv"); // output file for writing clean track paths
    std::ifstream primarySongsTable(databaseFile);
    std::string s_musiclibrarydirname = userconfig::getConfigEntry(1); // 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
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
                if (found!=std::string::npos) //&& (std::string::npos < str.length()))
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
            double tempLastPlayedDate{0.0};
            std::string strrandom;
            if (tokenCount == 17) {
                tempLastPlayedDate = std::atof(token.c_str()); //valid C++ syntax to convert a token string to a double.
                if (tempLastPlayedDate > (currSQLDate - 60.9999))
                {
                    if (tempLastPlayedDate > (currSQLDate - 10.9999))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL10TotTimeListened = *_sSQL10TotTimeListened + timeint;
                        ++*_sSQL10DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 11) && (tempLastPlayedDate > (currSQLDate - 20.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL20TotTimeListened = *_sSQL20TotTimeListened + timeint;
                        ++*_sSQL20DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 21) && (tempLastPlayedDate > (currSQLDate - 30.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL30TotTimeListened = *_sSQL30TotTimeListened + timeint                                ;
                        ++*_sSQL30DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 31) && (tempLastPlayedDate > (currSQLDate - 40.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL40TotTimeListened = *_sSQL40TotTimeListened + timeint;
                        ++*_sSQL40DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 41) && (tempLastPlayedDate > (currSQLDate - 50.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL50TotTimeListened = *_sSQL50TotTimeListened + timeint;
                        ++*_sSQL50DayTracksTot;}
                    if (tempLastPlayedDate <= (currSQLDate - 51) && (tempLastPlayedDate > (currSQLDate - 60.9999)))
                    {int timeint = std::stoi(tempTokenTrackTime);
                        *_sSQL60TotTimeListened = *_sSQL60TotTimeListened + timeint;
                        ++*_sSQL60DayTracksTot;}
                }
            }
            // TOKEN PROCESSING - COLS 12, 29
            // Then, check the current line for the GroupDesc (rating code, col 29), which is
            // then used to increment the song quantity and song time accordingly
            // Cols 12 and 29 are for song time and GroupDesc (rating code)
            // Collect statistical data for times and qtys of each rating category
            if (tokenCount == 29 && token == "0") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode0MsTotTime = *_srCode0MsTotTime + timeint;
                ++*_srCode0TotTrackQty;
            }
            if (tokenCount == 29 && token == "1") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode1MsTotTime = *_srCode1MsTotTime + timeint;
                ++*_srCode1TotTrackQty;
            }
            if (tokenCount == 29 && token == "3") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode3MsTotTime = *_srCode3MsTotTime + timeint;
                ++*_srCode3TotTrackQty;
            }
            if (tokenCount == 29 && token == "4") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode4MsTotTime = *_srCode4MsTotTime + timeint;
                ++*_srCode4TotTrackQty;
            }
            if (tokenCount == 29 && token == "5") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode5MsTotTime = *_srCode5MsTotTime + timeint;
                ++*_srCode5TotTrackQty;
            }
            if (tokenCount == 29 && token == "6") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode6MsTotTime = *_srCode6MsTotTime + timeint;
                ++*_srCode6TotTrackQty;
            }
            if (tokenCount == 29 && token == "7") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode7MsTotTime = *_srCode7MsTotTime + timeint;
                ++*_srCode7TotTrackQty;
            }
            if (tokenCount == 29 && token == "8") {
                int timeint = std::stoi(tempTokenTrackTime);
                *_srCode8MsTotTime = *_srCode8MsTotTime + timeint;
                ++*_srCode8TotTrackQty;
            }
            ++ tokenCount;
        }
        outf << str << "\n"; // The string is valid, write to clean file
        //str.clear();
        ++ stringCount;
        //std::cout << "getcleanlib: ran to 144. string count is: "<< stringCount<< std::endl;

    }
    //str.clear();
    //str.str(std::string());

    primarySongsTable.close();   // Close files opened for reading and writing
    outf.close();
}
