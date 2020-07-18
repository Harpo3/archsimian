#ifndef GETPLAYLIST_H
#define GETPLAYLIST_H
#include <iostream>
#include <vector>
#include <QString>
#include "utilities.h"

int musicLibraryDirLen(QString &s_musiclibrarydirname);

std::vector<std::string> getPlaylistVect(std::string fileName);

void getPlaylist(QString &s_defaultPlaylist, const QString &s_musiclibrarydirname, QString &s_musiclibshortened, bool s_topLevelFolderExists);

void exportPlaylistToWindows(int &s_musicdirlength, QString &s_mmPlaylistDir, QString &s_winDriveLtr, QString &s_musiclibrarydirname, std::string &playlistpath);

#endif // GETPLAYLIST_H
