#include <QApplication>
#include <QDir>
#include <iostream>
#include <fstream>
#include <sstream>
#include "utilities.h"
#include "constants.h"
#include <set>


// Function to calculate which artists need to be excluded from the current track selection
void getArtistExcludes()
{
    if (Constants::kVerbose) std::cout << "getArtistExcludes function started." << std::endl;
    std::fstream filestrinterval;
    filestrinterval.open ("ratedabbr.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "getArtistExcludes: Error opening ratedabbr.txt file after it was created in child process." << std::endl;}
    std::string ratedlibrary = "ratedabbr.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "getArtistExcludes: Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream playlistPosList("playlistposlist.txt"); // output file for writing ratedabbr.txt with added artist intervals
    std::string str1; // store the string for ratedabbr.txt
    std::string playlistPosition; // Custom1 variable from ratedabbr.txt
    std::string selectedArtistToken; // Artist variable from ratedabbr.txt
    std::map<std::string,int> countMap; // Create a map for two types, string and int
    std::vector<std::string>plvect;
    plvect.reserve(5000);
    // Outer loop: iterate through ratedSongsTable in the file "ratedabbr.txt" then use str1 to add Col 36 to ratedabbr.txt
    while (std::getline(ratedSongsTable, str1))
    {  // Declare variables applicable to all rows
        std::istringstream iss(str1); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, ','))
        {
            if ((tokenCount == 1) && (token != "0"))
            {playlistPosition = token;} // playlist position, exclude if not in playlist
            if (tokenCount == 3)  {selectedArtistToken = token;}//  artist is selected
            ++ tokenCount;
        }
        if ((playlistPosition != "0") && (playlistPosition != "Custom1")){
            std::string commatxt{","};
            std::string vectorstring;
            vectorstring.append(selectedArtistToken).append(commatxt).append(playlistPosition);
            plvect.push_back(vectorstring);}
        playlistPosition = "0";
    }
    std::sort (plvect.begin(), plvect.end());
    for(const auto & i : plvect)
    {
        playlistPosList << i << "\n";
    }
    plvect.shrink_to_fit();
    ratedSongsTable.close(); // Close ratedabbr.txt and output file
    playlistPosList.close();
}

// Function to create the artistexcludes file
void getExcludedArtists(const int &s_playlistSize)
{
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::fstream filestrinterval;
    int s_playlistPosition;
    filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "getArtistExcludes: Error opening ratedabbr.txt file ." << std::endl;}
    std::string ratedlibrary = appDataPathstr.toStdString()+"/ratedabbr.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "getArtistExcludes: Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    StringVector2D ratedabbrVec = readCSV(appDataPathstr.toStdString()+"/ratedabbr.txt");
    ratedabbrVec.reserve(50000);
    std::vector<std::string>histvect; // vector to collect excluded artists from 'outside the playlist'
    std::vector<std::string> artistExcludesVec;// vector to collect all excluded artists
    std::ofstream playlistPosList(appDataPathstr.toStdString()+"/playlistposlist.txt"); // output file for writing ratedabbr.txt with added artist intervals
    // Clear ratedabbr2 if file exists
    std::ofstream ofs; //open the cleanedplaylist file for writing with the truncate option to delete the content.
    ofs.open(appDataPathstr.toStdString()+"/ratedabbr2.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    std::ofstream ratedabbr2(appDataPathstr.toStdString()+"/ratedabbr2.txt"); // output file for writing ratedabbr.txt with added artist intervals
    std::string str1; // store the string for ratedabbr.txt
    //std::string playlistPosition; // Custom1 variable for playlistposlist.txt
    std::string selectedArtistToken; // Artist variable from ratedabbrVec
    std::string tokenLTP; // LastPlayed Date in SQL Time from ratedabbrVec
    std::string ratingCode; // Rating code from ratedabbrVec
    std::string songLength; // Song length from ratedabbrVec
    std::string artistInterval; // Artist interval from ratedabbrVec
    std::string albumID; // Album ID from ratedabbrVec
    std::string path;
    //open playlist to read in position number of each track
    std::fstream playList;
    playList.open (appDataPathstr.toStdString()+"/cleanedplaylist.txt");
    if (playList.is_open()) {playList.close();}
    else {std::cout << "getArtistExcludes: Error opening cleanedplaylist.txt file." << std::endl;}
    std::string playlist = appDataPathstr.toStdString()+"/cleanedplaylist.txt"; // now we can use it as input file
    std::ifstream playlistTable(playlist);
    if (!playlistTable.is_open())
    {
        std::cout << "getArtistExcludes: Error opening playlistTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    int playlistCount = 0;
    int songCount = 0;
    static std::string song;
    while (getline(playlistTable,song)){
        ++playlistCount;
        s_playlistPosition = s_playlistSize-playlistCount + 1; // playlist position is most recently added (to bottom)
        std::string txtPos;
        std::string pathinlib;
        std::string artistinvec;
        txtPos = std::to_string(s_playlistPosition);
        //std::cout << " PL POS: "<<s_playlistPosition  << " " << song <<std::endl;
        for(auto & i : ratedabbrVec){ // read each row element into the variables needed
            tokenLTP = i[Constants::kColumn0];
            ratingCode = i[Constants::kColumn1];
            selectedArtistToken = i[Constants::kColumn2];
            pathinlib = i[Constants::kColumn3];
            songLength = i[Constants::kColumn4];
            artistInterval = i[Constants::kColumn5];
            albumID = i[Constants::kColumn6];
            trim_cruft(pathinlib);
            trim_cruft(song);
            pathinlib.compare(song);
            // histvect - if path in vector row does not match a playlist entry, push to extended
            // play history vector
            if ((song != pathinlib) && (songCount == 1)){
                std::string commatxt{","};
                std::string vectorstring;
                vectorstring.append(tokenLTP).append(commatxt).append(selectedArtistToken).append(commatxt).append(songLength)
                        .append(commatxt).append(ratingCode).append(commatxt).append(artistInterval).append(commatxt).append(albumID);
                histvect.push_back(vectorstring);
                continue;
            }
            // ratedabbrVec - if path in vector row matches a playlist entry, remove zero from col 7 and emplace
            // playlist position number
            if (song == pathinlib){
                for(int j=0;j<20;j++){
                    i.pop_back();
                    i.emplace_back(txtPos);
                }
            }
            if (song == pathinlib){
                //compare playlist position to size of playlist and interval
                 if (s_playlistPosition < std::stoi(artistInterval)){ //add artist to exclude list
                    artistExcludesVec.push_back(selectedArtistToken);}
                }
            ++songCount;
        }
    }
    std::sort (histvect.begin(), histvect.end());
    std::reverse (histvect.begin(), histvect.end());

    std::string strbuild;
    std::sort (ratedabbrVec.begin(), ratedabbrVec.end());

    // Writes a 2D vector of strings to a file, each row with comma separated values
    // This file (ratedabbr2.txt) will be later used to select next track for playlist (total rating times, and current playlist position)
    for ( const auto &row : ratedabbrVec )
    {
        bool first = true;
        for ( const auto &s : row ) {
        if (first){ratedabbr2 << s;}
        else {ratedabbr2 <<','<< s;}
        first = false;
        }
        ratedabbr2 << std::endl;
    }
    std::vector<std::string> new_histvect;
    new_histvect.reserve(50000);
    new_histvect = histvect;
    for (std::size_t i = 0 ;  i < new_histvect.size(); i++){ // write new vector to "playlistposlist.txt"
        playlistPosList << new_histvect[i] << "," << i + 1 + static_cast<unsigned long>(s_playlistSize) << "\n";
    }
    playlistPosList.close();
    // Calculate the extended playlist to find infrequent artists played within their interval value (ex. postion 452 and interval 487)
    std::ofstream artistExcList(appDataPathstr.toStdString()+"/artistexcludes.txt"); // output file for writing final exclude list
    StringVector2D finalhistvec = readCSV(appDataPathstr.toStdString()+"/playlistposlist.txt"); // open "playlistposlist.txt" as 2D vector finalhistvec
    for (auto & i : finalhistvec){
        for(int j=0;j<6;j++){
            //std::cout << i[4] << ", " <<finalhistvec[i][5] << std::endl;
            if (std::stoi(i[4]) > std::stoi(i[5])){ // if playlist pos [6] is less than interval [5],
                //artistExcList << finalhistvec[i][1] << std::endl;
                //std::cout << finalhistvec[i][1] << ", "<< finalhistvec[i][4] << ", " <<finalhistvec[i][5]<< std::endl;
                artistExcludesVec.push_back(i[1]); // then pushback artist (finalhistvec[i][1]) to artistExcludesVec
            }
        }
    }
    // Compile all of the entries of the artistExcludesVec, sort and remove duplicates, then write the vector to the excluded artists file
    std::vector<std::string>fsexclvec;
    fsexclvec.reserve(10000);
    fsexclvec = artistExcludesVec;
    std::vector<std::string>::iterator ip;
    std::sort (fsexclvec.begin(), fsexclvec.end());
    std::set<std::string> s;
    unsigned long myvecsz = fsexclvec.size();
    for (unsigned long i = 0; i < myvecsz; ++i) s.insert(fsexclvec[i]);
    fsexclvec.assign (s.begin(), s.end());
    for (ip = fsexclvec.begin(); ip != fsexclvec.end(); ++ip) {
        artistExcList << *ip << "\n";
    }
    ratedSongsTable.close(); // Close ratedabbr.txt and output file
    ratedabbr2.close();
    artistExcList.close();
    //remove("playlistposlist.txt");
}
