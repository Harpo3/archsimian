#include <sstream>
#include "userconfig.h"
#include "lastplayeddays.h"
#include "getplaylist.h"

void getRatedTable()
{
    std::fstream filestr2;
    filestr2.open ("cleanlib.dsv");
    if (filestr2.is_open()) {filestr2.close();}
    else {std::cout << "getRatedTable: Error opening cleanlib.dsv file after it was created in child process." << std::endl;}
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

    // Run function getPlaylistVector to read playlist into a vector, and then to populate the Custom1 field
    std::vector<std::string> plStrings;
    plStrings = getPlaylistVect("cleanedplaylist.txt");

    // Make sure the playlist vector size is accurate
    if(plStrings.size() != static_cast<std::vector<int>::size_type>(plStrings.size()))
    {
        std::cerr << "getRatedTable: Error in plStrings vector size!" << std::endl;
    }
    unsigned long playlistSize = plStrings.size(); // total number of tracks in playlist minus one (index starts with 0)
    //
    //  Outer loop: iterate through rows of cleanedSongsTable
    //
    while (std::getline(cleanedSongsTable, str))
    {   // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        std::string tempTokenStarRating; //used to filter rows where star rating is zero;
        std::string tempinPlaylist{"0"}; //used to identify whether a track is in the playlist; if so add position #, else 0.
        int tokenCount{0}; //token count is the number of delimiter characters within str
        std::string strrandom; // stores a random number as a string variable
        skiprow=false; //sets whether a given str is output to the rated.dsv file

        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, '^'))
        {
            // TOKEN PROCESSING - COL 8 (delimiter # 7)
            // Compare the file path stored in vector plStrings to each token path; if match,
            //  set tempinPlaylist = index number in vector + 1 (which is the playlist position)
            //, if no match set tempinPlaylist to 0. All values set in text. Using the index number,
            // it will be subtracted from playlistSize to express 'tracks since added' to the playlist.
            // So, track number 470 on the playlist will be recorded as 1 (playlistSize + 1 - tempinPlaylist)
            if (tokenCount == 8)
            {
                unsigned long plcount{0};
                unsigned long plindex{0}; // variable for vector index number (starts with 0)
                for(size_t count=0;count<playlistSize; ++count)
                    if (token != plStrings[plcount])
                        plcount++;
                    else if (token == plStrings[plcount])
                    {
                        std::vector<std::string>::iterator it = std::find(plStrings.begin(), plStrings.end(), token);
                        // Get index of element from iterator
                        long plindex = std::distance(plStrings.begin(), it);
                        tempinPlaylist = std::to_string(playlistSize-plindex); // Sets playlist position as
                        continue;                                              // 'tracks ago' from last track added.
                    }
                    else {tempinPlaylist = "0";}
                if (tempinPlaylist == std::to_string(playlistSize-plindex)){token = tempinPlaylist;}
            }

            // TOKEN PROCESSING - COL 13 (delimiter # 12)
            // Store the star rating (col 13) in the tempTokenStarRating text variable
            if (tokenCount == 13) {tempTokenStarRating = token;}

            // TOKEN PROCESSING - COL 17 (delimiter #15 -text- delimiter #16)
            // Evaluates whether a lastplayed date (col 17) is zero, if so, replace with a random date
            if (tempTokenStarRating != "0" && tokenCount == 17 && token == ("0.0"))
                //&& tempTokenStarRating != "0")// generate a random lastplayed date if its current
                //  value is "0" unless track has a zero star rating
            {
                // Process a function to generate a random date 30-500 days ago then save to a string
                double rndresult{0.0};
                int intconvert;
                rndresult = getNewRandomLPDate(rndresult);
                if (rndresult == 0.0)
                {std::cout << "getRatedTable: Error obtaining random number at row: " << token << std::endl;}
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

            // TOKEN PROCESSING - COL 18 - Output 'in playlist' status from tempinPlaylist
            // calculated in col 8 and write it to col 18 (Custom1)
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
                // Unless it is the row header, insert the tempinPlaylist bool value (as text) to Custom1
                if (token != "Custom1") {
                    str.insert(myspot1,tempinPlaylist);}
            }

            // TOKEN PROCESSING - COL 19 (TBD) Add code to output artist data from Custom2 (Col 19) if it is a rated track [if (tempTokenStarRating != "0"]
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
            outfrated << str << "\n"; // The string is valid, write to clean file
        }
    }
    ++ stringCount;
    // Close files opened for reading and writing
    cleanedSongsTable.close();
    outfrated.close();
    plStrings.shrink_to_fit();
}
