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
void removeLinuxPlaylistFile();
bool mapFunc(const std::string &a, const std::string &b);
void splitsort(const std::string &s, std::string &mycombinedtext, std::string &selectedLPLSQLDate);
void syncAudaciousLog();
#endif // EXPORTTOANDROIDAIMP_H
