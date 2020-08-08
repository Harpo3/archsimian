#ifndef EXPORTTOANDROIDAIMP_H
#define EXPORTTOANDROIDAIMP_H
#include <string>
#include <QString>

void syncPlaylistWithSyncthing();
void removeMP3sSyncthing();
void testid3tag();
void getLastPlayedDates(QString &s_androidpathname);
double logdateconversion(std::string chkthis);
void updateCleanLibDates();
#endif // EXPORTTOANDROIDAIMP_H
