#ifndef GETPLAYLIST_H
#define GETPLAYLIST_H
#include <iostream>
#include <vector>
#include <QString>

std::vector<std::string> getPlaylistVect(std::string fileName);
void getPlaylist(const QString &s_defaultPlaylist, const QString &s_musiclibrarydirname);
int cstyleStringCount(std::string path);
void getWindowsDriveLtr(QString &s_defaultPlaylist, QString *s_winDriveLtr);
#endif // GETPLAYLIST_H
