#ifndef PLAYLISTFUNCTIONS_H
#define PLAYLISTFUNCTIONS_H
#include <string>

void getExcludedArtists(const long &s_histCount, const int &s_playlistSize);

int ratingCodeSelected(double &s_ratingRatio3, double &s_ratingRatio4, double &s_ratingRatio5,
                       double &s_ratingRatio6, double &s_ratingRatio7, double &s_ratingRatio8, int &s_rCode1TotTrackQty, int &repeatFreqCode1);
std::string selectTrack(int &s_ratingNextTrack, std::string *s_selectedTrackPath);

#endif // PLAYLISTFUNCTIONS_H
