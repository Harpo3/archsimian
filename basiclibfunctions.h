#ifndef BASICLIBFUNCTIONS_H
#define BASICLIBFUNCTIONS_H
#include <vector>
#include <string>

typedef std::vector<std::string> StringVector;
typedef std::vector<StringVector> StringVector2D;
using StringVector = std::vector<std::string>;
using StringVector2D = std::vector<StringVector>;

StringVector2D readCSV(std::string filename);
bool recentlyUpdated(std::string *_smmBackupDBDir);
std::string removeSpaces(std::string str);
std::string getChgdDSVStr(std::vector<std::string> const &input,std::string chgdString);
std::string getChgdDirStr(std::vector<std::string> const &input,std::string chgdString);
void getLibrary();
void getDBStats(int *_srCode0TotTrackQty,int *_srCode0MsTotTime,int *_srCode1TotTrackQty,int *_srCode1MsTotTime,
                 int *_srCode3TotTrackQty,int *_srCode3MsTotTime,int *_srCode4TotTrackQty,int *_srCode4MsTotTime,
                 int *_srCode5TotTrackQty,int *_srCode5MsTotTime,int *_srCode6TotTrackQty,int *_srCode6MsTotTime,
                 int *_srCode7TotTrackQty,int *_srCode7MsTotTime,int *_srCode8TotTrackQty,int *_srCode8MsTotTime,
                 int *_sSQL10TotTimeListened,int *_sSQL10DayTracksTot,double *_sSQL20TotTimeListened,
                 int *_sSQL20DayTracksTot,double *_sSQL30TotTimeListened,int *_sSQL30DayTracksTot,double *_sSQL40TotTimeListened,
                 int *_sSQL40DayTracksTot,double *_sSQL50TotTimeListened,int *_sSQL50DayTracksTot,double *_sSQL60TotTimeListened,
                 int *_sSQL60DayTracksTot);
void getArtistAdjustedCount(double *_syrsTillRepeatCode3factor,double *_syrsTillRepeatCode4factor,double *_syrsTillRepeatCode5factor,
                            double *_syrsTillRepeatCode6factor,double *_syrsTillRepeatCode7factor,double *_syrsTillRepeatCode8factor,
                            int *_srCode3TotTrackQty,int *_srCode4TotTrackQty,int *_srCode5TotTrackQty,
                            int *_srCode6TotTrackQty,int *_srCode7TotTrackQty,int *_srCode8TotTrackQty);
void buildDB();
#endif // BASICLIBFUNCTIONS_H
