#ifndef GETPLAYLIST_H
#define GETPLAYLIST_H
#include <iostream>
#include <vector>
#include <QString>

int musicLibraryDirLen(QString &s_musiclibrarydirname);
std::vector<std::string> getPlaylistVect(std::string fileName);
void getPlaylist(const QString &s_defaultPlaylist, const QString &s_musiclibrarydirname);
int cstyleStringCount(std::string path);
void getWindowsDriveLtr(QString &s_defaultPlaylist, QString *s_winDriveLtr);
void exportPlaylistToWindows(int &s_musicdirlength, QString &s_mmPlaylistDir, QString &s_defaultPlaylist, QString &s_winDriveLtr, QString &s_musiclibrarydirname);
#endif // GETPLAYLIST_H