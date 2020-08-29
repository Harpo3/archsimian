#include <QStandardPaths>
#include <QDir>
#include <fstream>
#include <sstream>
#include "basiclibfunctions.h"
#include "getplaylist.h"
#include "constants.h"
#include "utilities.h"

// Function to populate four variables used to determine rating code 1 track selection in function getNewTrack
void code1stats(int *_suniqueCode1ArtistCount, int *_scode1PlaylistCount, int *_slowestCode1Pos, std::string *_sartistLastCode1){
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::vector<std::string>code1artistsvect;
    std::string selectedArtistToken; // Artist variable
    std::string str; // store the string for ratedabbr2.txt
    std::string ratingCode;
    std::string playlistPos;
    int posint{Constants::kMaxHighestCode1Pos};
    std::fstream filestrinterval;
    filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {
        std::cout << "code1stats: Error opening ratedabbr2.txt file." << std::endl;
        Logger ("code1stats: Error opening ratedabbr2.txt file.");
    }
    std::string ratedlibrary = appDataPathstr.toStdString()+"/ratedabbr2.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()) {
        std::cout << "code1stats: Error opening ratedabbr2.txt." << std::endl;
        Logger ("code1stats: Error opening ratedabbr2.txt file.");
        std::exit(EXIT_FAILURE);
    }
    while (std::getline(ratedSongsTable, str)) {  // Declare variables applicable to all rows
        std::istringstream iss(str); // str is the string of each row
        std::string token; // token is the contents of each column of data
        int tokenCount{0}; //token count is the number of delimiter characters within str
        while (std::getline(iss, token, ',')) {
            if (tokenCount == Constants::kColumn1) {ratingCode = token;}// store rating variable
            if (tokenCount == Constants::kColumn2) {selectedArtistToken = token;} //Selected artist token
            if (tokenCount == Constants::kColumn7)  {
                playlistPos = token;
                if ((ratingCode == "1") && (playlistPos != "0")){
                    ++*_scode1PlaylistCount;
                }
            }
            ++ tokenCount;
        }
        if ((ratingCode == "1") && (playlistPos != "0")){//rating code 1 track is in the playlist
            posint = std::stoi(playlistPos);
            if (*_slowestCode1Pos > posint){
                *_slowestCode1Pos = posint;
                *_sartistLastCode1 = selectedArtistToken;
            }
        }
        if ((std::find(code1artistsvect.begin(), code1artistsvect.end(), selectedArtistToken) == code1artistsvect.end()) && (ratingCode == "1")) {
            code1artistsvect.push_back(selectedArtistToken);
            ++*_suniqueCode1ArtistCount;
        }
    }
}

// Function used to select a rating code 1 track
void getNewTrack(std::string &s_artistLastCode1, std::string *s_selectedCode1Path){
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::string returntrack;
    std::fstream filestrinterval;
    filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "getNewTrack: Error opening ratedabbr2.txt file." << std::endl;}
    std::string ratedlibrary = appDataPathstr.toStdString()+"/ratedabbr2.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable(ratedlibrary);
    if (!ratedSongsTable.is_open()) {
        std::cout << "getNewTrack: Error opening ratedabbr2.txt." << std::endl;
        Logger ("getNewTrack: Error opening ratedabbr2.txt file.");
        std::exit(EXIT_FAILURE);
    }
    std::string str1; // Store the string for ratedabbr2.txt
    std::string tokenLTP;
    std::string ratingCode;
    std::string selectedArtistToken;
    std::string songPath;
    std::string playlistPos;
    std::string albumID;
    std::vector<std::string>code1tracksvect; // New vector to store all code 1 selections
    // Outer loop: iterate through ratedSongsTable in the file "ratedabbr2.txt"
    while (std::getline(ratedSongsTable, str1)) {  // Declare variables applicable to all rows
        std::istringstream iss(str1); // str is the string of each row
        std::string token;
        int tokenCount{0}; // Token count is the number of delimiter characters within str
        // Inner loop: iterate through each column (token) of row
        while (std::getline(iss, token, ',')) {
            if ((tokenCount == Constants::kColumn0) && (token != "0")) {tokenLTP = token;} // LastPlayedDate in SQL Time
            if (tokenCount == Constants::kColumn1) {ratingCode = token;}
            if (tokenCount == Constants::kColumn2) {selectedArtistToken = token;}
            if (tokenCount == Constants::kColumn3) {songPath = token;}
            if (tokenCount == Constants::kColumn6) {albumID = token;}
            if (tokenCount == Constants::kColumn7)  {playlistPos = token;}
            ++ tokenCount;
        }
        if ((ratingCode == "1") && (playlistPos == "0") &&(selectedArtistToken != s_artistLastCode1)) {
            // If a code 1 track is not in the playlist and not the last artist selected,
            // add to vector used to return track path to s_selectedCode1Path
            std::string commatxt{","};
            std::string vectorstring;
            vectorstring.append(tokenLTP).append(commatxt).append(selectedArtistToken).append(commatxt).append(songPath).append(commatxt).append(playlistPos);
            code1tracksvect.push_back(vectorstring);}
    }
    std::sort (code1tracksvect.begin(), code1tracksvect.end());
    std::string fullstring = code1tracksvect.front();
    std::vector<std::string> splittedStrings = split(fullstring, ',');
    returntrack = splittedStrings[2];
    *s_selectedCode1Path = returntrack;
    //Write/append s_selectedTrackPath to the cleanedplaylist.txt file.
    std::ofstream playlist(appDataPathstr.toStdString()+"/cleanedplaylist.txt",std::ios::app);
    playlist << *s_selectedCode1Path << "\n";
    playlist.close();
}
