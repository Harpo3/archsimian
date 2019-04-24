#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <fstream>
#include <sstream>
#include <iostream>
#include "utilities.h"
#include "constants.h"


/*
This function runs after the function ratingCodeSelected, if enabled by the user.
Iterate through tracks in ratedabbr2.txt starting from oldest playlist position to newest
for each artist in the artistalbmexcls.txt file. Sends the IDs to a text file excludeids.txt
*/

void getAlbumIDs(){
    //if (Constants::kVerbose) std::cout << "Starting selectTrack function. Rating for next track is " << s_ratingNextTrack << std::endl;
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::fstream filestrartists;
    filestrartists.open (appDataPathstr.toStdString()+"/selalbmexcl.txt");
    if (filestrartists.is_open()) {filestrartists.close();}
    else {std::cout << "Error opening selalbmexcl.txt file." << std::endl;}
    std::string artistalbmexcls1 = appDataPathstr.toStdString()+"/selalbmexcl.txt";
    std::ifstream artistTable1(artistalbmexcls1);
    if (!artistTable1.is_open()) {
        std::cout << "getAlbumIDs: Error opening selalbmexcl.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream trimmedlist(appDataPathstr.toStdString()+"/finalids.txt");
    std::string str1;
    std::string tokenLTP;
    std::string selectedArtistToken;
    std::string ratingCode;
    std::string playlistPos;
    std::string albumID;
    while (std::getline(artistTable1, str1)) {
        std::string str2;
        //search filestrartists2 for matching string
        std::fstream filestrartists2;
        std::fstream filestrinterval;
        filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr2.txt");
        if (filestrinterval.is_open()) {filestrinterval.close();}
        else {std::cout << "getAlbumIDs: Error opening ratedabbr2.txt file (42)." << std::endl;}
        std::string ratedlibrary = appDataPathstr.toStdString()+"/ratedabbr2.txt";
        std::ifstream ratedSongsTable(ratedlibrary);
        if (!ratedSongsTable.is_open()) {
            std::cout << "getAlbumIDs: Error opening ratedabbr2.txt (46)." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        // set variables used to compare element values in ratedabbr2 against this str1
        int finalplaylistPos = Constants::kMaxFinalPlaylistPos;
        int finaltokenLTP{0};
        std::string finalAlbumID{"0"};
        while (std::getline(ratedSongsTable, str2)) {  // Declare variables applicable to all rows
            std::istringstream iss(str2); // str is the string of each row
            std::string token; // token is the contents of each column of data
            int tokenCount{0}; //token count is the number of delimiter characters within str
            while (std::getline(iss, token, ',')) {
                // TOKEN PROCESSING - COL 0
                if ((tokenCount == Constants::kColumn0) && (token != "0")) {tokenLTP = token;}// get LastPlayedDate in SQL Time
                // TOKEN PROCESSING - COL 1
                if (tokenCount == Constants::kColumn1) {ratingCode = token;}// store rating variable
                // TOKEN PROCESSING - COL 2
                if (tokenCount == Constants::kColumn2) {selectedArtistToken = token;} //Selected artist token
                // TOKEN PROCESSING - COL 6
                if (tokenCount == Constants::kColumn6) {albumID = token;} // store album ID variable
                // TOKEN PROCESSING - COL 7
                if (tokenCount == Constants::kColumn7)  {
                    if (token != "0"){
                        int tmpint = std::stoi(token);
                        if (finalplaylistPos > tmpint){playlistPos = token;}
                        else playlistPos = "0";
                    }
                    else playlistPos = "0";
                }
                ++ tokenCount;
            }
            int foundmatch{0};
            int tmppos{0};
            foundmatch = stringMatch(selectedArtistToken, str1);// Check whether the artist in ratedabbr2 matches artist in selalbmexcl.txt
            if ((playlistPos != "0") && (foundmatch == 1)){// If the artist matches, get lowest playlist position and save albumID associated with it
                // if the matched artist is in the playlist or extended count, get lowest playlist position
                tokenLTP = "0"; // reset lasttime played when/if playlist entry found
                tmppos = std::stoi(playlistPos);
                finalplaylistPos = tmppos;
                finalAlbumID = albumID;
            }
            // If the artist matches, but no playlist position, get most recent lastplayed date and save albumID associated with it
            if ((playlistPos == "0") && (foundmatch == 1)){
                // if the matched artist is not in the playlist or extended count, check last time played if playlist position is zero
                int tmpltp = std::stoi(tokenLTP);
                if ((finaltokenLTP < tmpltp) && (finalplaylistPos == Constants::kMaxFinalPlaylistPos)) {
                    finaltokenLTP = tmpltp; // update finalLTP if greater than existing finalLTP
                    finalAlbumID = albumID; // save albumID only if not already found in playlist
                }
            }
        }
        ratedSongsTable.close();
        trimmedlist << finalAlbumID << std::endl;
    }
    artistTable1.close();
    trimmedlist.close();
}

/*
At time of track selection (if user selected album variety), use the file artistalbmexcls.txt
and artistexcludes.txt to create a list of artists subject to album variety screen but excluding
those already on the excluded artists list. Output to a temp file selalbmexcl.txt
*/
void getTrimArtAlbmList(){
    std::fstream filestrartists;
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    filestrartists.open (appDataPathstr.toStdString()+"/artistalbmexcls.txt");
    if (filestrartists.is_open()) {filestrartists.close();}
    else {std::cout << "getTrimArtAlbmList: Error opening artistalbmexcls.txt file." << std::endl;}
    std::string artistalbmexcls1 = appDataPathstr.toStdString() + "/artistalbmexcls.txt"; // now we can use it as input file
    std::ifstream artistTable1(artistalbmexcls1);
    if (!artistTable1.is_open()) {
        std::cout << "getTrimArtAlbmList: Error opening artistalbmexcls.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ofstream trimmedlist(appDataPathstr.toStdString()+"/selalbmexcl.txt");
    std::string str1;
    while (std::getline(artistTable1, str1)) {
        //search filestrartists2 for matching string
        std::fstream filestrartists2;
        filestrartists2.open (appDataPathstr.toStdString()+"/artistexcludes.txt");
        if (filestrartists2.is_open()) {filestrartists2.close();}
        else {std::cout << "Error opening artistexcludes.txt file." << std::endl;}
        std::string artistexcludes2 = appDataPathstr.toStdString()+"/artistexcludes.txt"; // now we can use it as input file
        std::ifstream artistTable2(artistexcludes2);
        if (!artistTable2.is_open()) {
            std::cout << "getArtCompare: Error opening artistexcludes.txt." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        int foundmatch{0};
        foundmatch = matchLineinIfstream(artistTable2, str1);
        artistTable2.close();
        if (foundmatch == 0)
            trimmedlist << str1 << std::endl;
    }
    trimmedlist.close();
    artistTable1.close();
}
