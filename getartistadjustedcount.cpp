#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

void getArtistAdjustedCount(double *_syrsTillRepeatCode3factor,double *_syrsTillRepeatCode4factor,double *_syrsTillRepeatCode5factor,
                            double *_syrsTillRepeatCode6factor,double *_syrsTillRepeatCode7factor,double *_syrsTillRepeatCode8factor,
                            int *_srCode3TotTrackQty,int *_srCode4TotTrackQty,int *_srCode5TotTrackQty,
                            int *_srCode6TotTrackQty,int *_srCode7TotTrackQty,int *_srCode8TotTrackQty)
{
    //std::cout << "Working on artist counts and factors. This will take a few seconds...";
    bool customArtistID{true};    
    std::ifstream rated;  // First ensure rated.dsv is ready to open
    rated.open ("rated.dsv");
    if (rated.is_open()) {rated.close();}
    else {std::cout << "getArtistAdjustedCount: Error opening rated.dsv file." << std::endl;}
    std::string ratedSongsTable = "rated.dsv";    // Now we can use it as input file
    std::ifstream SongsTable(ratedSongsTable);    // Open rated.dsv as ifstream
    if (!SongsTable.is_open())
    {
        std::cout << "getArtistAdjustedCount: Error opening SongsTable." << std::endl;
        std::exit(EXIT_FAILURE); // Otherwise, quit
    }
    std::string str; // Create ostream file to log artists and duplicates; dups will be used to create a vector with # tracks per artist
    std::ofstream outartists("artists2.txt"); // output file for writing artists list with dups
    //  Outer loop: iterate through rows of SongsTable
    while (std::getline(SongsTable, str))
    {   // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        std::string selectedArtistToken; //used to filter rows where track has a playlist position;
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, '^'))
        {
            // TOKEN PROCESSING - COL 1
            if ((tokenCount == 1) && (customArtistID == false)) {selectedArtistToken = token;}// if 'non-custom' artist is selected use this code
            // TOKEN PROCESSING - COL 19
            if ((tokenCount == 19) && (customArtistID == true)) {selectedArtistToken = token;}// if custom artist grouping is selected use this code
            ++ tokenCount;
        }
        outartists << selectedArtistToken << "\n"; // Write artist to clean file
    }
    SongsTable.close();    // Close files opened for reading and writing. SongsTable will be reopened shortly as ratedSongsTable2
    outartists.close();
    std::map<std::string, int> countMap; // Create a map for two types, string and int
    std::vector<std::string> artists; // Now create vector
    std::ifstream myfile("artists2.txt"); // Input file for vector
    std::ofstream artistList("artists.txt"); // output file for writing artists list without dups using vector
    std::string line;
    while ( std::getline(myfile, line) ) { //std::cout << "Vector Size is now " << artists.size() << " lines." << std::endl;
        if ( !line.empty() )
            artists.push_back(line);
    }
    for (auto & elem : artists)  // Iterate over the vector and store the frequency of each element in map
    {
        auto result = countMap.insert(std::pair<std::string, int>(elem, 1));
        if (result.second == false)
            result.first->second++;
    }
    // Iterate over the map
    for (auto & elem : countMap)
    {
        // If frequency count is greater than 0 then its element count is captured
        if (elem.second > 0)
        {
            artistList << elem.first << "," << elem.second << "\n";
        }
    }
    myfile.close();
    artistList.close();
    if( remove( "artists2.txt" ) != 0 )
        perror( "getArtistAdjustedCount: Error deleting file" );
    std::ifstream rated2;  // Ensure rated.dsv is ready to open
    rated2.open ("rated.dsv");
    if (rated2.is_open()) {rated2.close();}
    else {std::cout << "getArtistAdjustedCount: Error rated2 opening rated.dsv file." << std::endl;}
    std::string ratedSongsTable2 = "rated.dsv"; // now we can use it as input file    
    std::ifstream artists2;  // Next ensure artists.txt is ready to open
    artists2.open ("artists.txt");
    if (artists2.is_open()) {artists2.close();}
    else {std::cout << "getArtistAdjustedCount: Error artists2 opening artists.txt file ." << std::endl;}
    std::string artistsTable2 = "artists.txt"; // now we can use it as input file    
    std::ifstream artistcsv(artistsTable2); // Open artists.txt as ifstream
    if (!artistcsv.is_open())
    {
        std::cout << "Error opening artistcsv." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // store the string for artists.txt
    std::string str2; // store the string for rated.dsv    
    std::ofstream outartists2("artistsadj.txt"); // Create ostream file to collect artists and adjusted counts
    std::string currentArtist;
    int currentArtistCount{0};
    // Outer loop: iterate through artist, track count in the file "artists.txt"
    // For each artist i with a track count more than 1, store a temp variable vSelArtist for iterating
    // If artist count is 1, set adjusted count to 1 and store adjCount of 1 in Custom 3
    while ( std::getline(artistcsv, str1) )
    {
        double interimAdjCount{0.0};
        int tokenArtistsCount{0};
        std::istringstream issArtists(str1); // str is the string of each row
        std::string tokenArtist; // token is the content of each column of data in artists.txt
        int countdown{0};
        // First loop: iterate through each column (token) of current row of artists.txt
        // to get the artist name and number of tracks to find in rated.dsv
        while (std::getline(issArtists, tokenArtist, ','))
        {
            countdown = currentArtistCount + 1;
            // TOKEN PROCESSING - COL 0
            if (tokenArtistsCount == 0) {currentArtist = tokenArtist;}
            //std::cout << "Current artist from artists.txt is " << currentArtist << std::endl;
            // TOKEN PROCESSING - COL 1
            if (tokenArtistsCount == 1) {currentArtistCount = std::stoi(tokenArtist);}
            ++tokenArtistsCount;
        }
        countdown = currentArtistCount;
        // Second loop uses rated.dsv
        // open rated.dsv and vector for artist track count and calculate adjusted track count, and write to Custom3
        // Open rated.dsv as ifstream
        std::ifstream SongsTable2(ratedSongsTable2);
        if (!SongsTable2.is_open())
        {
            std::cout << "getArtistAdjustedCount: Error opening SongsTable2." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        while (std::getline(SongsTable2, str2) && countdown != 0) //Check every row until all artist's tracks found
        {   // Declare variables applicable to all rows
            std::istringstream iss(str2); // str is the string of each row
            std::string token; // token is the contents of each column of data
            std::string selectedArtistToken; //used to store artist name read from token;
            std::string selectedRating; //used to store rating for current row;
            int tokenCount{0}; //token count is the number of delimiter characters within str
            bool artistMatch = false;
            // Inner loop within second loop: iterate through each column (token) of row to find and match artist
            while (std::getline(iss, token, '^'))
            {
                //artistMatch = false;
                // TOKEN PROCESSING - COL 1 if 'non-custom' artist is selected use this token
                if ((tokenCount == 1) && (customArtistID == false)) {
                    selectedArtistToken = token;
                    if (currentArtist == selectedArtistToken)   {
                        //set temp variable to check when the rating token is checked next
                        artistMatch = true;
                        -- countdown;
                    }
                }
                // TOKEN PROCESSING - COL 19 if custom artist is selected use this token
                if ((tokenCount == 19) && (customArtistID == true)) {
                    selectedArtistToken = token;
                    if (currentArtist == selectedArtistToken) {
                        //set temp variable to check when the rating token is checked next
                        artistMatch = true;
                        -- countdown;
                        //                       std::cout << " countdown is now " << std::to_string(countdown) << std::endl;
                    }
                }
                // TOKEN PROCESSING - COL 29 get rating and store for current row
                if (tokenCount == 29) {
                    if (artistMatch == 1) {
                        selectedRating = token;
                        // Now evaluate the rating using factors and calculate adjusted track value
                        // Increment adjusted values as each track is found using the rating factor stats collected
                        if (selectedRating == "1") {interimAdjCount = interimAdjCount + 1;}
                        if (selectedRating == "3") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode3factor;}
                        if (selectedRating == "4") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode4factor;}
                        if (selectedRating == "5") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode5factor;}
                        if (selectedRating == "6") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode6factor;}
                        if (selectedRating == "7") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode7factor;}
                        if (selectedRating == "8") {interimAdjCount = interimAdjCount + *_syrsTillRepeatCode8factor;}
                    }
                }
                // Increment to the next column of row in rated.dsv
                ++ tokenCount;
            }
            continue; // Resume rated.dsv next row, beginning with Col 0
        }
        // Completed all rows of rated.dsv
        double s_totalAdjRatedQty = (*_syrsTillRepeatCode3factor * *_srCode3TotTrackQty)+(*_syrsTillRepeatCode4factor * *_srCode4TotTrackQty)
                    + (*_syrsTillRepeatCode5factor * *_srCode5TotTrackQty) +(*_syrsTillRepeatCode6factor * *_srCode6TotTrackQty)
                    +(*_syrsTillRepeatCode7factor * *_srCode7TotTrackQty) + (*_syrsTillRepeatCode8factor * *_srCode8TotTrackQty);
        SongsTable2.close(); // Must close rated.dsv here so it can reopen for the next artist on the artists.txt file
        if (interimAdjCount < currentArtistCount) {interimAdjCount = currentArtistCount;} // Adjusted count must be at least one if there is one track or more
        double currentArtistFactor = (interimAdjCount / s_totalAdjRatedQty); //percentage of total adjusted tracks
        int availInterval = int(1 / currentArtistFactor);

        // Write artist, count, adjusted count, artist factor, and repeat interval to the output file if not the header row
        if ((currentArtist != "Custom2") && (currentArtist != "Artist")){
            outartists2 << currentArtist << "," << int(currentArtistCount) << "," << int(interimAdjCount) <<
                           "," << currentArtistFactor << "," << availInterval << std::endl;
        }
        // Resume with next artist on the artists.txt file
    }
    // All entries in the artists.txt file completed and adjusted values written to new file. Close files opened for reading and writing
    artistcsv.close();    
    outartists2.close();
    artists.shrink_to_fit();
    if( remove( "artists.txt" ) != 0 )
        perror( "getArtistAdjustedCount: Error deleting artists.txt file" );
}
