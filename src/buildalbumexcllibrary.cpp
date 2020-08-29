#include <QStandardPaths>
#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QString>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include "utilities.h"
#include"constants.h"

template <typename T>
void findDuplicates(std::vector<T> & vecOfElements, std::map<T, int> & countMap)
{
    // Iterate over the vector and store the frequency of each element in map
    for (auto & elem : vecOfElements)
    {
        auto result = countMap.insert(std::pair<std::string, int>(elem, 1));
        if (!result.second)
            result.first->second++;
    }
    // Remove the elements from Map which has 1 frequency count
    for (auto it = countMap.begin() ; it != countMap.end() ;)
    {
        if (it->second == 1)
            it = countMap.erase(it);
        else
            it++;
    }
}

// Function to identify which albums should be excluded based on the album of the last track played for each artist
// meeting the user criteria selected
void buildAlbumExclLibrary(const int &s_minalbums, const int &s_mintrackseach, const int &s_mintracks)
{
    // Open artadj.txt (contains total track count) to find which artists meet the minimum
    // number of total tracks, then run 'for' loop on each artist that meets and populate
    // map vector trackCountMap with the results
    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    StringVector2D artAdjVec1 = readCSV(appDataPathstr.toStdString()+"/artistsadj.txt");
    std::fstream artadj;
    artadj.open (appDataPathstr.toStdString()+"/artistsadj.txt");
    if (artadj.is_open()) {artadj.close();}
    else {std::cout << "buildAlbumExclLibrary: Error opening artadj.txt file ." << std::endl;}
    std::string artscreen = appDataPathstr.toStdString()+"/artistsadj.txt"; // now we can use it as input file
    std::ifstream artScreenTable(artscreen);
    if (!artScreenTable.is_open())
    {
        std::cout << "buildAlbumExclLibrary: Error opening artScreenTable." << std::endl;
        Logger ("buildAlbumExclLibrary: Error opening artScreenTable.");
        std::exit(EXIT_FAILURE);
    }
    std::map<std::string,int> trackCountMap; // Create a total track count map with string and int, for output
    std::string artScreenLine;
    std::string selectedArtistToken; // Artist variable from artScreenTable [0]
    std::string totalTracksToken; // Total tracks count variable from artScreenTable [1]
    while (getline(artScreenTable,artScreenLine)){
        for(auto & i : artAdjVec1){ // Read each row element into the variables needed
            selectedArtistToken = i[0];
            totalTracksToken = i[1];
            int tmpttt = std::stoi(totalTracksToken);
            if (tmpttt >= s_mintracks) {
                trackCountMap.insert(std::make_pair(selectedArtistToken,tmpttt));
            }
        }
    }
    artScreenTable.close(); // Close ifstream file, leave vector trackCountMap open
    std::ofstream totTrackCountList(appDataPathstr.toStdString()+"/tmpcount1.txt"); // tmp output file for writing vector
    //iterate and output to temp txt file
    auto it = trackCountMap.begin();
    while(it != trackCountMap.end())
    {
        totTrackCountList<<it->first<<std::endl;
        it++;
    }
    totTrackCountList.close();
    std::vector <std::string> albIDVect;
    // Add ostream for 2nd tmp file
    std::ofstream dupAlbumIDList(appDataPathstr.toStdString()+"/tmpcount2.txt"); // tmp output file for writing vector
    // Now, for each artist in the tmp file, iterate through the library file and push the artist and Album ID to a vector
    // including all the duplicate values for album ID
    std::string artist;
    int new_artistCount = cstyleStringCount(appDataPathstr.toStdString()+"/tmpcount1.txt");
    // Two read files, first the library: minAlbumsScreenTable
    std::fstream minalbadj;
    minalbadj.open (appDataPathstr.toStdString()+"/cleanlib.dsv");
    if (minalbadj.is_open()) {minalbadj.close();}
    else {
        std::cout << "buildAlbumExclLibrary: Error opening cleanlib.dsv." << std::endl;
        Logger ("buildAlbumExclLibrary: Error opening cleanlib.dsv.");
    }
    std::string minAlbumsScreen = appDataPathstr.toStdString()+"/cleanlib.dsv"; // Now we can use it as input file
    // Then, the previous tmp file with the list of artists from the first screen: minAlbumsScreenTable1
    std::fstream totTrackCountList1;
    totTrackCountList1.open (appDataPathstr.toStdString()+"/tmpcount1.txt");
    if (totTrackCountList1.is_open()) {totTrackCountList1.close();}
    else {
        std::cout << "buildAlbumExclLibrary: Error opening tmpcount1.txt." << std::endl;
        Logger ("buildAlbumExclLibrary: Error opening tmpcount1.txt.");
    }
    std::string minAlbumsScreen1 = appDataPathstr.toStdString()+"/tmpcount1.txt"; // Now we can use it as input file
    std::ifstream minAlbumsScreenTable1(minAlbumsScreen1);
    if (!minAlbumsScreenTable1.is_open())
    {
        std::cout << "buildAlbumExclLibrary: Error opening minAlbumsScreenTable1." << std::endl;
        Logger ("buildAlbumExclLibrary: Error opening minAlbumsScreenTable1.");
        std::exit(EXIT_FAILURE);
    }
    // Outer loop (artist list)
    std::string str;
    int h = 0;
    while(h < new_artistCount){ // Iterate through length of artist list, store each artist as variable artist
        std::string str1;
        std::istringstream iss(artist);
        std::getline(minAlbumsScreenTable1,artist);
        std::string selectedArtistTmpToken = artist;
        std::string selectedArtistAlbToken; // Artist variable from minAlbumsScreenTable [19], skip line if this token is empty ""
        std::string AlbumIDToken; // Total albums count variable from minAlbumsScreenTable [2], need to count unique values
        std::string ratingCodeToken; // Rating code to only count rated tracks [29]
        std::ifstream minAlbumsScreenTable(minAlbumsScreen);
        if (!minAlbumsScreenTable.is_open())
        {
            std::cout << "buildAlbumExclLibrary: Error opening minAlbumsScreenTable." << std::endl;
            Logger ("buildAlbumExclLibrary: Error opening minAlbumsScreenTable.");
            std::exit(EXIT_FAILURE);
        }
        std::getline(minAlbumsScreenTable, str); //skip first row
        while (std::getline(minAlbumsScreenTable, str)) {   // Outer loop: // for each artist str, iterate through each row of library
            std::string token1;
            std::istringstream iss1(str1);
            std::getline(minAlbumsScreenTable, str1);
            int tokenCount1{0};
            while (std::getline(iss1, token1, '^')) { // Inner loop: iterate through each column (token) of row
                if (tokenCount1 == Constants::kColumn2) {AlbumIDToken = token1;}
                if (tokenCount1 == Constants::kColumn19) {selectedArtistAlbToken = token1;}
                if (tokenCount1 == Constants::kColumn29) {ratingCodeToken = token1;}
                ++tokenCount1;
            }
            trim_cruft(selectedArtistAlbToken);
            trim_cruft(selectedArtistTmpToken);
            if (selectedArtistAlbToken == selectedArtistTmpToken){
                std::string commatxt{","};
                std::string vectorstring;
                vectorstring.append(selectedArtistAlbToken).append(commatxt).append(AlbumIDToken);
                albIDVect.push_back(vectorstring);
            }
        }
        ++h;
        minAlbumsScreenTable.close();
    }    
    for (const auto & i : albIDVect){ // Write vector to 2nd tmp file
        dupAlbumIDList << i << "\n";}    
    dupAlbumIDList.close(); // Close two read files after 2nd tmp file is written
    minAlbumsScreenTable1.close();
    std::vector<std::string> mymap;
    std::string tmpfile = appDataPathstr.toStdString()+"/tmpcount2.txt";
    std::string stra;
    std::ifstream mytmpfile(tmpfile);
    std::ofstream tmp3{appDataPathstr.toStdString()+"/tmpcount3.txt"};
    while (std::getline(mytmpfile, stra))
    {        
        if(!stra.empty()) // Line contains string of length > 0 then save it in multimap
            mymap.push_back(stra);
    }
    std::map<std::string, int> duplicateElements;
    findDuplicates(mymap, duplicateElements);
    for (auto & elem : duplicateElements){
        if (elem.second >= s_mintrackseach){ // This is the minimum tracks per album variable
            tmp3 << elem.first << "," << elem.second << std::endl;}
    }
    tmp3.close();
    std::string tmpfile1 = appDataPathstr.toStdString()+"/tmpcount3.txt";
    std::string str1;
    std::ifstream mytmpfile1(tmpfile1);
    std::ofstream tmp4{appDataPathstr.toStdString()+"/tmpcount4.txt"};
    while (std::getline(mytmpfile1, str1))
    {
        std::size_t found = str1.find_first_of(',');
        std::string trimmed1 = str1.substr(0,found);
        tmp4 << trimmed1 <<std::endl;
    }
    tmp4.close();
    std::vector<std::string> mymap1;
    std::string tmpfile2 = appDataPathstr.toStdString()+"/tmpcount4.txt";
    std::string str2;
    std::ifstream mytmpfile2(tmpfile2);
    std::ofstream finallist {appDataPathstr.toStdString()+"/artistalbmexcls.txt"};
    while (std::getline(mytmpfile2, str2))
    {        
        if(!str2.empty()){ // Line contains string of length > 0 then save it in multimap
            mymap1.push_back(str2);
        }
    }
    std::map<std::string, int> duplicateElements1;
    findDuplicates(mymap1, duplicateElements1);
    for (auto & elem : duplicateElements1){
        if (elem.second >= s_minalbums){  // This is the minimum albums variable
            finallist << elem.first << std::endl;
        }
    }
    finallist.close();
    mytmpfile.close();
    mytmpfile2.close();
    // Remove temp files generated
    removeAppData("tmpcount1.txt");
    removeAppData("tmpcount2.txt");
    removeAppData("tmpcount3.txt");
    removeAppData("tmpcount4.txt");
}
