#include <QApplication>
#include <QDir>
#include <iostream>
#include <fstream>
#include <sstream>
#include "utilities.h"
#include "constants.h"
#include <set>


// Function to create the artistexcludes file
void getExcludedArtists(const int &s_playlistSize)
{
    // Using a vector of ratedabbr and reading cleanedplaylist, calculate each playlist position,
    // then output each position, as duplicate of ratedabbr but with position column added, to playlistposlist.txt
    // ratedabbr2
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::fstream filestrinterval;
    int s_playlistPosition;
    filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "getExcludedArtists: Error opening ratedabbr.txt file ." << std::endl;}
    std::string ratedlibrary = appDataPathstr.toStdString()+"/ratedabbr.txt";
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "getExcludedArtists: Error opening ratedSongsTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    StringVector2D ratedabbrVec = readCSV(appDataPathstr.toStdString()+"/ratedabbr.txt");
    ratedabbrVec.reserve(50000);
    std::vector<std::string>histvect; // Vector to collect excluded artists not in the playlist but whose intervals exclude them anyway
    std::vector<std::string> artistExcludesVec;// Vector to collect all excluded artists
    std::ofstream playlistPosList(appDataPathstr.toStdString()+"/playlistposlist.txt"); // Output file for writing ratedabbr.txt with added artist intervals
    std::ofstream ofs; //open the ratedabbr2 file for writing with the truncate option to delete the content.
    ofs.open(appDataPathstr.toStdString()+"/ratedabbr2.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    std::ofstream ratedabbr2(appDataPathstr.toStdString()+"/ratedabbr2.txt"); // output file for writing ratedabbr.txt with added artist intervals
    std::string str1; // store the string for ratedabbr.txt line
    std::string selectedArtistToken; // Artist variable from ratedabbrVec
    std::string tokenLTP; // LastPlayed Date in SQL Time from ratedabbrVec
    std::string ratingCode; // Rating code from ratedabbrVec
    std::string songLength; // Song length from ratedabbrVec
    std::string artistInterval; // Artist interval from ratedabbrVec
    std::string albumID; // Album ID from ratedabbrVec
    std::fstream playList; //open cleanedplaylist to read in position number of each track
    playList.open (appDataPathstr.toStdString()+"/cleanedplaylist.txt");
    if (playList.is_open()) {playList.close();}
    else {std::cout << "getExcludedArtists: Error opening cleanedplaylist.txt file." << std::endl;}
    std::string playlist = appDataPathstr.toStdString()+"/cleanedplaylist.txt";
    std::ifstream playlistTable(playlist);
    if (!playlistTable.is_open())
    {
        std::cout << "getExcludedArtists: Error opening playlistTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    int playlistCount = 0;
    int songCount = 0;
    static std::string song;
    while (getline(playlistTable,song)){ // Read cleanedplaylist to determine playlist position of each entry
        int tempsize{0};
        ++playlistCount;
        if (s_playlistSize-playlistCount + 1 == 0) { tempsize = 1;}
        s_playlistPosition = s_playlistSize-playlistCount + tempsize + 1; // Playlist position is most recently added (to bottom)
        std::string txtPos;
        std::string pathinlib;
        std::string artistinvec;
        txtPos = std::to_string(s_playlistPosition);
        for(auto & i : ratedabbrVec){ // Read each row element from ratedabbrVec to compare with each playlist entry
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
            // Histvect - if path in any of the ratedabbrVec rows does not match a playlist entry, push to histvect, which is the
            // vector used to collect excluded artists not in the playlist (but whose intervals exclude them anyway)
            if ((song != pathinlib) && (songCount == 1)){
                std::string commatxt{","};
                std::string vectorstring;
                vectorstring.append(tokenLTP).append(commatxt).append(selectedArtistToken).append(commatxt).append(songLength)
                        .append(commatxt).append(ratingCode).append(commatxt).append(artistInterval).append(commatxt).append(albumID);
                histvect.push_back(vectorstring);
                continue;
            }
            //for (auto i = histvect.begin(); i != histvect.end(); ++i)
            //    std::cout << *i << ','<< std::endl;
            // RatedabbrVec - if path in ratedabbrVec row matches a cleanedplaylist entry, remove zero from col 7 and emplace
            // playlist position number in ratedabbrVec
            if (song == pathinlib){
                for(int j=0;j<20;j++){
                    i.pop_back();
                    i.emplace_back(txtPos);
                }
            }
            // After matching and emplacing position number to ratedabbrVec, compare match playlist position with the associated
            // artist interval. If the position number is less than the interval, push to artistExcludesVec
            if (song == pathinlib){
                 if (s_playlistPosition < std::stoi(artistInterval)){
                    artistExcludesVec.push_back(selectedArtistToken);}
                }
            ++songCount;
        }
    }
    std::sort (histvect.begin(), histvect.end());
    std::reverse (histvect.begin(), histvect.end());
    std::sort (ratedabbrVec.begin(), ratedabbrVec.end());
    // Write ratedabbrVec to ratedabbr2.txt as a comma separated value file (ratedabbr2.txt). This includes playlist posistion
    //  numbers, but does not include excluded artists not in the playlist (but whose intervals exclude them anyway)
    for ( const auto &row : ratedabbrVec )
    {
        bool first = true;
        for ( const auto &s : row ) {
        if (first){ratedabbr2 << s;} // For first element, do not add a comma before element
        else {ratedabbr2 <<','<< s;} // Every element after, add a comma before element
        first = false;
        }
        ratedabbr2 << std::endl;
    }
    ratedabbr2.close();
    // Make a copy of histvect as new_histvect (why?) and use to write a comma separated value file (playlistposlist.txt).
    std::vector<std::string> new_histvect;
    new_histvect.reserve(50000);
    new_histvect = histvect;
    for (std::size_t i = 0 ;  i < new_histvect.size(); i++){
        playlistPosList << new_histvect[i] << "," << i + 1 + static_cast<unsigned long>(s_playlistSize) << "\n";
    }
    playlistPosList.close();
    // Calculate the extended play history for infrequently played artists, but who were played within their high interval
    // value (ex. position 452 and interval 487). Where found, add them to the artistExcludesVec
    std::ofstream artistExcList(appDataPathstr.toStdString()+"/artistexcludes.txt"); // Output file for writing final exclude list
    StringVector2D finalhistvec = readCSV(appDataPathstr.toStdString()+"/playlistposlist.txt"); // Open "playlistposlist.txt" as 2D vector finalhistvec
    for (auto & i : finalhistvec){
        for(int j=0;j<6;j++){
            if (stoi(i[4]) > stoi(i[6])){ // If playlist pos [6] is less than interval [4],
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
    artistExcList.close();
    ratedabbrVec.shrink_to_fit();
    new_histvect.shrink_to_fit();
    histvect.shrink_to_fit();
    artistExcludesVec.shrink_to_fit();
    fsexclvec.shrink_to_fit();
    finalhistvec.shrink_to_fit();
}
