#include <iostream>
#include <fstream>
#include <constants.h>
#include <stdio.h>


void writeSQLFile()
{
    std::string str1{".separator \"^\""};
    std::string str2{".header on"};
    std::string str3{".once libtable.dsv"};
    std::string str4{"SELECT"};
    std::string str5{"ID,Artist,IDAlbum,Album,AlbumArtist,DiscNumber,TrackNumber,"
                     "SongTitle,SongPath,Year,Genre,FileLength,SongLength,Rating,"
                     "Bitrate,BPM,PlayCounter,LastTimePlayed,Custom1,Custom2,"
                     "Custom3,Custom4,Custom5,DateAdded,Publisher,Lyricist,"
                     "Conductor,Remixer,InvolvedPeople,GroupDesc,OrigYear,Tempo,"
                     "Mood,Occasion,Quality,SkipCount FROM songs"};
    const std::string sqlpathdirname = getenv("HOME");
    std::ofstream sqlfile(sqlpathdirname + Constants::sqlFileName); // exportMMTable.sql
    sqlfile << str1 << "\n";
    sqlfile << str2 << "\n";
    sqlfile << str3 << "\n";
    sqlfile << str4 << "\n";
    sqlfile << str5 << "\n";
    sqlfile.close();
    std::string sqlfile2(sqlpathdirname + Constants::sqlFileName);
    std::ifstream sqlfile3;
    sqlfile3.open(sqlfile2.c_str());
    if (sqlfile3.is_open()) {sqlfile3.close();}
    else {std::cout << "Error opening SQL file after it was created by writeSQL()." << std::endl;}
}

void removeSQLFile()
{
    const std::string sqlpathdirname = getenv("HOME");
    std::string sqlfile(sqlpathdirname + Constants::sqlFileName);
    remove (sqlfile.c_str());// remove exportMMTable.sql from home directory
}