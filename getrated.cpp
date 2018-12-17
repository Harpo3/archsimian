#include <sstream>
#include <unistd.h>
#include "archsimian.h"
#include "dependents.h"
#include "userconfig.h"
#include "runbashapp.h"
#include "lastplayeddays.h"
#include "constants.h"

void getRatedTable()
{
//***************************************************************

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
            unsigned long myspot = 0;
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

//************************************************************
}
