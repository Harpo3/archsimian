#ifndef EXPORTTOANDROIDAIMP_H
#define EXPORTTOANDROIDAIMP_H
#include <string>
#include <QString>
#include <id3/tag.h>
#include <id3/misc_support.h>

void syncPlaylistWithSyncthing();
void ShowKnownFrameInfo();
void readTags(const std::string& musicFile);
void removeMP3sSyncthing();
void getLastPlayedDates(QString s_androidpathname);
double logdateconversion(std::string chkthis);
void updateCleanLibDates();
void removeLinuxPlaylistFile();
void syncAudaciousLog();
void updateChangedTagRatings();
void PrintInformation(ID3_Tag &myTag, std::string *popmToken, std::string *ratingToken);
#endif // EXPORTTOANDROIDAIMP_H
