#include <sstream>
#include "userconfig.h"
#include "lastplayeddays.h"
#include "getplaylist.h"

void getRatedTable()
{
    std::fstream filestr2;
    filestr2.open ("cleanlib.dsv");
    if (filestr2.is_open()) {filestr2.close();}
    else {std::cout << "Error opening cleanlib.dsv file after it was created in child process." << std::endl;}
    std::string cleanlibrary = "cleanlib.dsv"; // now we can use it as input file
    std::ofstream outfrated("rated.dsv"); // output file for writing clean track paths
    //std::ofstream artistlist("artistlist.txt");
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
    std::vector<std::string> plStrings;
    getPlaylistVect("cleanedplaylist.txt", plStrings);
    while (std::getline(cleanedSongsTable, str))
    {   // Outer loop: iterate through rows of cleanedSongsTable
        // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        std::string tempTokenStarRating; //used to filter rows where star rating is zero;
        bool tempinPlaylist{0}; //used to track whether a row is in the current playlist
        int tokenCount{0}; //token count is the number of delimiter characters within str
        std::string strrandom; // stores a random number as a string variable
        skiprow=false; //sets whether a given str is output to the rated.dsv file

        while (std::getline(iss, token, '^'))
        {
            // Inner loop: iterate through each column (token) of row
            // TOKEN PROCESSING - COL 8 (delimiter # 7)
            // Check the playlist paths against each token path, if a match tempinPlaylist = true, else false
            if (tokenCount == 8)
            {
                unsigned long plcount{0};
                for(int count=0;count<460; ++count)
                    if (token != plStrings[plcount])
                        plcount++;
                    else if (token == plStrings[plcount])
                    {
                        tempinPlaylist = true;
                        continue;
                    }
                    else {tempinPlaylist = false;}

                if (tempinPlaylist == true)
                {//std::cout << "Playlist status for " << token << " is true: " << tempinPlaylist << std::endl;
                token = tempinPlaylist;}
                //if (tempinPlaylist == false)
                //{std::cout << "Playlist status for " << token << " is false, even though plStrings has " << plStrings[plcount] << std::endl;}

            }


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

                 // Add code to output playlist status of token path of col 8 and place in col 18

            if (tokenCount == 18)
            {
                signed int poscount1 = 0;
                unsigned long myspot1 = 0;
                std::size_t found1 = str.find_first_of("^");
                while (found1!=std::string::npos)
                {
                    if (poscount1 == 18) {myspot1 = found1;}
                    found1=str.find_first_of("^",++found1);
                    ++poscount1;
                }
                //std::cout << "myspot1 captured: " << myspot1 << std::endl;
                //std::cout << "plfound captured: " << found1 << std::endl;
                std::string textBool("false");
                if (tempinPlaylist == false) {textBool = "false";}
                else {textBool = "true";}
                str.insert(myspot1,textBool);
                //if (textBool == "true") {std::cout << "tempinPlaylist is true, setting for : " << str << std::endl;}
            }

            // Add code to output artist data from Custom 2 (Col 19) if it is a rated track [if (tempTokenStarRating != "0"]
 //           if (tokenCount == 19 && tempTokenStarRating != "0") {
                //
                //std::cout << "Artist value captured: " << token << std::endl;
                //artistlist << token << std::endl; // The string is valid, write to clean file
 //           }

            // TOKEN PROCESSING - COLS 13, 29
            // Then, check the current line for the GroupDesc (rating code, col 29), which is
            // then compared with the col 13 star rating temp variable
            // If there is no rating and a "0" is assigned as a rating code,
            // do not write the row to new file
            if (tokenCount == 29 && token == "0") {
                // do not write row - set skip variable skiprow
                skiprow = true;}
            //***************************************
            if (tokenCount == 29 && token == "")
            {
                std::string newstr;
                if (tempTokenStarRating == "100") newstr = "3";
                if ((tempTokenStarRating == "90") || (tempTokenStarRating == "80")) newstr = "4";
                if (tempTokenStarRating == "70") newstr = "5";
                if (tempTokenStarRating == "60") newstr = "6";
                if (tempTokenStarRating == "50") newstr = "7";
                if ((tempTokenStarRating == "30") || (tempTokenStarRating == "40")) newstr = "8";
                if (tempTokenStarRating == "20") newstr = "1";
                if (tempTokenStarRating == "10") newstr = "0";
                signed int poscount2 = 0;
                unsigned long myspot2 = 0;
                std::size_t found2 = str.find_first_of("^");
                while (found2!=std::string::npos)
                {
                    if (poscount2 == 28) {myspot2 = found2+1;}
                    found2=str.find_first_of("^",++found2);
                    ++poscount2;
                }
                str.replace(myspot2,0,newstr);
            }
            //*********************************************

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
    //artistlist.close();
}
