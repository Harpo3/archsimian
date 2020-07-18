#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <fstream>
#include <sstream>
#include "basiclibfunctions.h"
#include "getplaylist.h"
#include "constants.h"
#include "utilities.h"

void setPlaylistLimitCount (int &selectedTrackLimitCode, int *s_playlistActualCntSelCode)
{
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::string strlc; // store the string for ratedabbr2.txt
    std::string ratingCodeFound2{""};
    std::string strLimitcode = std::to_string(selectedTrackLimitCode);
    int interimcount{0};
    //if (Constants::kVerbose) std::cout << "setPlaylistLimitCount function: strLimitcode: "
    //                                   <<strLimitcode <<". Starting function. "<< std::endl;
    std::string playlistPos{0};
    std::ifstream filestrinterval;
    filestrinterval.open (appDataPathstr.toStdString()+"/ratedabbr2.txt");
    if (filestrinterval.is_open()) {filestrinterval.close();}
    else {std::cout << "setPlaylistLimitCount: Error opening ratedabbr2.txt file." << std::endl;}
    std::string ratedlibrary2 = appDataPathstr.toStdString()+"/ratedabbr2.txt"; // now we can use it as input file
    std::ifstream ratedSongsTable2(ratedlibrary2);
    if (!ratedSongsTable2.is_open()) {
        std::cout << "setPlaylistLimitCount: Error opening ratedabbr2.txt." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    while (std::getline(ratedSongsTable2, strlc)) {  // Declare variables applicable to all rows
        std::istringstream iss2(strlc); // str is the string of each row
        std::string token2; // token is the contents of each column of data
        int tokenCount2{0}; //token count is the number of delimiter characters within str
        while (std::getline(iss2, token2, ',')) {
            if (tokenCount2 == Constants::kColumn1) {ratingCodeFound2 = token2;}// store rating variable
            if (tokenCount2 == Constants::kColumn7) {
                playlistPos = token2;
                if ((ratingCodeFound2 == strLimitcode ) && (playlistPos != "0")){ // 3 was
                    ++interimcount;
                }
            }
            ++ tokenCount2;
        }
        strlc.clear();
    }
    *s_playlistActualCntSelCode = interimcount;
    ratedSongsTable2.close();
}

