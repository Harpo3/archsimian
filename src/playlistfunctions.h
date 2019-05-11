#ifndef PLAYLISTFUNCTIONS_H
#define PLAYLISTFUNCTIONS_H
#include <string>
#include <vector>

//void getExcludedArtists(const int &s_playlistSize);
int ratingCodeSelected(double &s_ratingRatio3, double &s_ratingRatio4, double &s_ratingRatio5,
                       double &s_ratingRatio6, double &s_ratingRatio7, double &s_ratingRatio8);

std::string selectTrack(int &s_ratingNextTrack, std::string *s_selectedTrackPath, bool &s_includeAlbumVariety);

void code1stats(int *_suniqueCode1ArtistCount, int *_scode1PlaylistCount, int *_slowestCode1Pos, std::string *_sartistLastCode1);

#endif // PLAYLISTFUNCTIONS_H
