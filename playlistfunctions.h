#ifndef PLAYLISTFUNCTIONS_H
#define PLAYLISTFUNCTIONS_H
#include <string>
#include <vector>

void getExcludedArtists(const long &s_histCount, const int &s_playlistSize);

int ratingCodeSelected(double &s_ratingRatio3, double &s_ratingRatio4, double &s_ratingRatio5,
                       double &s_ratingRatio6, double &s_ratingRatio7, double &s_ratingRatio8);
std::string selectTrack(int &s_ratingNextTrack, std::string *s_selectedTrackPath);
std::vector<std::string> split(std::string strToSplit, char delimeter);
void code1stats(int *_suniqueCode1ArtistCount, int *_scode1PlaylistCount, int *_slowestCode1Pos, std::string *_sartistLastCode1);
void getNewTrack(std::string &s_artistLastCode1, std::string *s_selectedCode1Path);

#endif // PLAYLISTFUNCTIONS_H
