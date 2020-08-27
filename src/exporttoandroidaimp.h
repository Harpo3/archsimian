#ifndef EXPORTTOANDROIDAIMP_H
#define EXPORTTOANDROIDAIMP_H
#include <string>
#include <QString>
#include <id3/tag.h>
#include <id3/misc_support.h>

void syncPlaylistWithSyncthing();
void removeMP3sSyncthing();
void getLastPlayedDates(QString s_androidpathname);
double logdateconversion(std::string chkthis);
void updateCleanLibDates();
void removeLinuxPlaylistFile();
void syncAudaciousLog();
void updateChangedTagRatings();
#endif // EXPORTTOANDROIDAIMP_H
