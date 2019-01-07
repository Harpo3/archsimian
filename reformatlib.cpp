#include <sstream>
#include "userconfig.h"
#include "lastplayeddays.h"

void getReformattedTable()
{
    std::fstream filestr2;
    filestr2.open ("cleanlib.dsv");
    if (filestr2.is_open()) {filestr2.close();}
    else {std::cout << "Error opening cleanlib.dsv file after it was created in child process." << std::endl;}
    std::string cleanlibrary = "cleanlib.dsv"; // now we can use it as input file
    std::ofstream outfilereformat("cleanlibrfmt.dsv"); // output file for writing lastplayed for rated tracks
                                                                                    // without a lastplayed date, & delims/types reformatted
    std::ifstream cleanedSongsTable(cleanlibrary);
    //bool skiprow{false}; //filter for rated only, false means to include all rows

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
    while (std::getline(cleanedSongsTable, str))
    {   // Outer loop: iterate through rows of cleanedSongsTable
        // Declare variables applicable to all rows
        std::istringstream iss(str);
        std::string strnew;
        std::string token;
        std::string tempTokenStarRating; //used to filter rows where star rating is zero;
        int tokenCount{0}; //token count is the number of delimiter characters within the string
        std::string strrandom; // store random number generated in a text variable
        //bool skiprow=false;

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
//
//
// REMOVING THIS SECTION FOR REFORMATTED VERSION
//
//
            // TOKEN PROCESSING - COLS 13, 29
            // Then, check the current line for the GroupDesc (rating code, col 29), which is
            // then compared with the col 13 star rating temp variable
            // If there is no rating and a "0" is assigned as a rating code,
            // do not write the row to new file
//            if (tokenCount == 29 && token == "0") {
                // do not write row - set skip variable skiprow
                //skiprow = true;}
            //***************************************
//            if (tokenCount == 29 && token == "")
//            {
//                std::string newstr;
//                if (tempTokenStarRating == "100") newstr = "3";
//                if ((tempTokenStarRating == "90") || (tempTokenStarRating == "80")) newstr = "4";
//                if (tempTokenStarRating == "70") newstr = "5";
//                if (tempTokenStarRating == "60") newstr = "6";
//                if (tempTokenStarRating == "50") newstr = "7";
//                if ((tempTokenStarRating == "30") || (tempTokenStarRating == "40")) newstr = "8";
//                if (tempTokenStarRating == "20") newstr = "1";
//                if (tempTokenStarRating == "10") newstr = "0";
//                signed int poscount2 = 0;
//                unsigned long myspot2 = 0;
//                std::size_t found2 = str.find_first_of("^");
//                while (found2!=std::string::npos)
//                {
//                    if (poscount2 == 28) {myspot2 = found2+1;}
//                    found2=str.find_first_of("^",++found2);
//                    ++poscount2;
//                }
//                str.replace(myspot2,0,newstr);
//            }
            //*********************************************
            ++ tokenCount;
        }
//
//
// REMOVED SO ALL ROWS WILL PRINT TO FILE
//
//
        std::string nbs{",\""};
        std::string sbn{ "\","};
        std::string sbs{"\",\""};
        std::string nbn{","};
        std::string whichstr;
        int delimcount = 1;
        //signed int poscount = 0;
        unsigned long myspot = 0;
        std::size_t found = str.find_first_of("^");
        while (delimcount < 36)
        {
            if (delimcount == 1 || delimcount == 3 || delimcount == 10 || delimcount == 18 || delimcount == 24 || delimcount == 31)
            {
               whichstr=nbs;
               //std::cout << "whichstr is: " << whichstr << '\n';
               //std::cout << "Delimcount is now: " << delimcount << std::endl;
               // poscount = 0;
                found=str.find('^');
                //if (found!=std::string::npos)
                    //std::cout << "Delimiter found at: " << found << '\n';
                myspot = found+1;
                //found=str.find("^",++found);
                //std::cout << myspot << '\n';
                //if (delimcount == 18) --myspot;
                str.replace(myspot-1,1,whichstr);
                //else {str.replace(myspot,1,whichstr);}
                //std::cout << "Here is the revised string: " << str << std::endl;
                ++delimcount;
                ++myspot;
                continue;
            }
            else if (delimcount == 2 || delimcount == 9 || delimcount == 11|| delimcount == 23 || delimcount == 29|| delimcount == 35) {
                whichstr=sbn;
                //std::cout << "whichstr is: " << whichstr << '\n';
                //std::cout << "Delimcount is now: " << delimcount << std::endl;
    //            poscount = 0;
                found=str.find('^');
                //if (found!=std::string::npos)
                    //std::cout << "Delimiter found at: " << found << '\n';
                myspot = found+1;
                //found=str.find("^",++found);
                //std::cout << myspot << '\n';
                str.replace(myspot-1,1,whichstr);
                //std::cout << "Here is the revised string: " << str << std::endl;
                ++delimcount;
                ++myspot;
                continue;
            }
            else if (delimcount == 4 || delimcount == 5 || delimcount == 6 || delimcount == 7 || delimcount == 8 || delimcount == 19 || delimcount == 20||
                     delimcount == 21||delimcount == 22||delimcount == 25||delimcount == 26||delimcount == 27||delimcount == 28||
                     delimcount == 32||delimcount == 33||delimcount == 34) {
                whichstr=sbs;
                //std::cout << "whichstr is: " << whichstr << '\n';
                //std::cout << "Delimcount is now: " << delimcount << std::endl;
                //poscount = 0;
                found=str.find('^');
                //if (found!=std::string::npos)
                    //std::cout << "Delimiter found at: " << found << '\n';
                myspot = found+1;
                //found=str.find("^",++found);
                //std::cout << myspot << '\n';
                str.replace(myspot-1,1,whichstr);
                //std::cout << "Here is the revised string: " << str << std::endl;
                ++delimcount;
                ++myspot;
                continue;
            }
            else if (delimcount == 11 || delimcount == 12 || delimcount == 13 || delimcount == 14 || delimcount == 15 || delimcount == 16 || delimcount == 17 || delimcount == 30)
            {whichstr=nbn;
                //std::cout << "whichstr is: " << whichstr << '\n';
                //std::cout << "Delimcount is now: " << delimcount << std::endl;
                //poscount = 0;
                found=str.find('^');
                //if (found!=std::string::npos)
                    //std::cout << "Delimiter found at: " << found << '\n';
                myspot = found+1;
                //found=str.find("^",++found);
                //std::cout << myspot << '\n';
                str.replace(myspot-1,1,whichstr);
                //std::cout << "Here is the revised string: " << str << std::endl;
                ++delimcount;
                ++myspot;
                continue;
            }
            else {
                std::cout << "There was an error with the delimiter for the songs table.";
                break;
            }
            //std::cout << "Delimcount here is now: " << delimcount << std::endl;
        }
    std::cout << "Here is the revised string: " << str << std::endl;
        //if (skiprow == false) { // If the track is rated
            outfilereformat << str << "\n"; // The string is valid, write to clean file
       // }
    }
    ++ stringCount;
    // Close files opened for reading and writing
    cleanedSongsTable.close();
    outfilereformat.close();
}
