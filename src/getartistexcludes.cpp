#include <QApplication>
#include <QDir>
#include <iostream>
#include <fstream>
#include <sstream>
#include "utilities.h"
#include "constants.h"
#include <set>

// Functions to determine playlist positions and positions of earlier play history, then use it to determine excluded artists
// First function, getExcludedArtists, uses ratedabbr.txt to create ratedabbr2.txt adding playlist position numbers to last column.
// Second function, getExcludedArtistsRedux, uses ratedabbr2.txt to determine excluded artist both inside and outside the
// playlist. Creates artistexcludes.txt.

void getExcludedArtists(const int &s_playlistSize)
{
    // Using a vector of ratedabbr and read of cleanedplaylist, calculate each playlist position,
    // then output each position, as a duplicate of ratedabbr but with position number added, to both playlistposlist.txt
    // playlistposlist.txt and ratedabbr2
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();


    // If playlist size is zero, copy ratedabbr.txt to ratedabbr2.txt and return
    if (s_playlistSize == 0) { // If there is no playlist
        QString tempFileStr1 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/ratedabbr.txt";
        QString tempFileStr2 = QDir::homePath() + "/.local/share/" + QApplication::applicationName() + "/ratedabbr2.txt";
        QFile::copy(tempFileStr1,tempFileStr2);
        while (!QFile::copy(tempFileStr1,tempFileStr2))
        {
            QFile::remove(tempFileStr2);
        }
        return; // end function if no playlist entries exist.
    }
    // If there is a playlist with entries, continue
    std::fstream filestrinterval;
    int s_playlistPosition;
    filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {
        std::cout << "getExcludedArtists: Error opening ratedabbr.txt file." << std::endl;
        Logger ("getExcludedArtists: Error opening ratedabbr.txt file.");
    }
    std::string ratedlibrary = appDataPathstr.toStdString()+"/ratedabbr.txt";
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open())
    {
        std::cout << "getExcludedArtists: Error opening ratedSongsTable." << std::endl;
        Logger ("getExcludedArtists: Error opening ratedSongsTable.");
        std::exit(EXIT_FAILURE);
    }
    StringVector2D ratedabbrVec = readCSV(appDataPathstr.toStdString()+"/ratedabbr.txt");
    ratedabbrVec.reserve(50000);
    std::vector<std::string>histvect; // Vector to collect excluded artists not in the playlist but whose intervals exclude them anyway
    histvect.reserve(50000);
    std::vector<std::string> artistExcludesVec;// Vector to collect all excluded artists
    std::ofstream playlistPosList(appDataPathstr.toStdString()+"/playlistposlist.txt"); // Output file for writing ratedabbr.txt with added artist intervals
    std::ofstream ofs; //open the ratedabbr2 file for writing with the truncate option to delete the content.
    ofs.open(appDataPathstr.toStdString()+"/ratedabbr2.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    std::ofstream ratedabbr2(appDataPathstr.toStdString()+"/ratedabbr2.txt"); // ratedabbr2 is output file for ratedabbr.txt (w/ position numbers added)
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
    else {
        std::cout << "getExcludedArtists: Error opening cleanedplaylist.txt file." << std::endl;
        Logger ("getExcludedArtists: Error opening cleanedplaylist.txt file.");
    }
    std::string playlist = appDataPathstr.toStdString()+"/cleanedplaylist.txt";
    std::ifstream playlistTable(playlist);
    if (!playlistTable.is_open())
    {
        std::cout << "getExcludedArtists: Error opening playlistTable." << std::endl;
        Logger ("getExcludedArtists: Error opening playlistTable.");
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
    // Write (ratedabbrVec to) ratedabbr2.txt as a comma separated value file. This adds playlist posistion
    //  numbers, but does account for those excluded artists 'outside' the playlist (but whose intervals exclude them)
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
    ratedSongsTable.close(); // Close ratedabbr.txt and output file    
    ratedabbrVec.shrink_to_fit();
    histvect.shrink_to_fit();
    artistExcludesVec.shrink_to_fit();
}

void getExcludedArtistsRedux(const int &s_playlistSize, const int &s_histCount){
    // Using a vector of ratedabbr2 calculate availability for each artist in and outside of the playlist.
    // then output to artistexcludes.txt.
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::fstream filestrinterval;
    filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {
        std::cout << "getExcludedArtists: Error opening ratedabbr2.txt file." << std::endl;
        Logger ("getExcludedArtists: Error opening ratedabbr2.txt file.");
    }
    std::string ratedlibrary2 = appDataPathstr.toStdString()+"/ratedabbr2.txt";
    std::ifstream ratedSongsTable2(ratedlibrary2);
    if (!ratedSongsTable2.is_open())
    {
        std::cout << "getExcludedArtists: Error opening ratedSongsTable." << std::endl;
        Logger ("getExcludedArtists: Error opening ratedSongsTable.");
        std::exit(EXIT_FAILURE);
    }
    std::string selectedArtistToken2; // Artist variable from ratedabbrVec
    std::string artistInterval2; // Artist interval from ratedabbrVec
    std::string playlistposition2;
    std::vector<std::string> artistexclistVec;
    std::ofstream artistExcList2(appDataPathstr.toStdString()+"/artistexcludes.txt"); // Output file for writing final exclude list ,std::ios::app
    StringVector2D ratedabbrVec2 = readCSV(appDataPathstr.toStdString()+"/ratedabbr2.txt");
    ratedabbrVec2.reserve(50000);
    for(auto & i : ratedabbrVec2){ // Read each row element from ratedabbrVec2 to compare with each historical entry
        selectedArtistToken2 = i[Constants::kColumn2];
        artistInterval2 = i[Constants::kColumn5];
        playlistposition2 = i[Constants::kColumn7];
    }
    std::reverse (ratedabbrVec2.begin(), ratedabbrVec2.end()); // Sort starting with most recently played
    // Starting with s_playlistSize then increasing by 1 for each row, number the sequences of last played
    // entries, then compare with artist interval to determien availability.
    // If playlist pos is less than interval, output to excluded artist vector
    // startingplaylistPosition = s_playlistSize + 1;
    for (int i = 0; i < int(ratedabbrVec2.size()); i++) { // Look at all rows of ratedabbr2 up to the histCount limit
        for (int j = 0; j < int(ratedabbrVec2[i].size()); j++)
        // Consider entries that are not on the playlist, within the historical count, and count is less than interval
        if ((ratedabbrVec2[i][7] == "0")&&(i < s_histCount)&&((stoi(ratedabbrVec2[i][5]) > i+1+s_playlistSize))){
            artistexclistVec.push_back(ratedabbrVec2[i][2]);
        }
        else { // Otherwise, the row may be a playlist item if it has a playlist position number
            if (ratedabbrVec2[i][7] != "0"){
                if ((stoi(ratedabbrVec2[i][5]) > (stoi(ratedabbrVec2[i][7])))){
                    artistexclistVec.push_back(ratedabbrVec2[i][2]);
                }
            }
        }
    }
    // Sort excluded artist vector alphabetically
    std::sort(artistexclistVec.begin(), artistexclistVec.end());
    artistexclistVec.erase(std::unique(artistexclistVec.begin(),artistexclistVec.end()),artistexclistVec.end());
    //Now print excluded artist vector to artistExcList2
    for (const auto & i : artistexclistVec){
        artistExcList2 << i << "\n";}
    artistExcList2.close();
    ratedabbrVec2.shrink_to_fit();
    artistexclistVec.shrink_to_fit();
}
