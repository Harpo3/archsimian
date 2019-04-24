#ifndef BASICLIBFUNCTIONS_H
#define BASICLIBFUNCTIONS_H
#include <vector>
#include <QString>


void removeAppData(std::string str);
bool recentlyUpdated(const QString &s_mmBackupDBDir);
std::string removeSpaces(std::string str);
void getLibrary(const QString &s_musiclibrarydirname);
void getDBStats(int *_srCode0TotTrackQty,int *_srCode0MsTotTime,int *_srCode1TotTrackQty,int *_srCode1MsTotTime,
                 int *_srCode3TotTrackQty,int *_srCode3MsTotTime,int *_srCode4TotTrackQty,int *_srCode4MsTotTime,
                 int *_srCode5TotTrackQty,int *_srCode5MsTotTime,int *_srCode6TotTrackQty,int *_srCode6MsTotTime,
                 int *_srCode7TotTrackQty,int *_srCode7MsTotTime,int *_srCode8TotTrackQty,int *_srCode8MsTotTime,
                 int *_sSQL10TotTimeListened,int *_sSQL10DayTracksTot,double *_sSQL20TotTimeListened,
                 int *_sSQL20DayTracksTot,double *_sSQL30TotTimeListened,int *_sSQL30DayTracksTot,double *_sSQL40TotTimeListened,
                 int *_sSQL40DayTracksTot,double *_sSQL50TotTimeListened,int *_sSQL50DayTracksTot,double *_sSQL60TotTimeListened,
                 int *_sSQL60DayTracksTot);
void getSubset();
//void buildAlbumExclLibrary(const int &s_minalbums, const int &s_mintrackseach, const int &s_mintracks);
#endif // BASICLIBFUNCTIONS_H
