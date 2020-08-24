#ifndef PLAYLISTFUNCTIONS_H
#define PLAYLISTFUNCTIONS_H
#include <string>
#include <vector>

//void getExcludedArtists(const int &s_playlistSize);
int ratingCodeSelected(double &s_playlistPercentage3, double &s_playlistPercentage4, double &s_playlistPercentage5,
                       double &s_playlistPercentage6, double &s_playlistPercentage7, double &s_playlistPercentage8);

std::string selectTrack(int &s_ratingNextTrack, std::string *s_selectedTrackPath, bool &s_includeAlbumVariety);

void code1stats(int *_suniqueCode1ArtistCount, int *_scode1PlaylistCount, int *_slowestCode1Pos, std::string *_sartistLastCode1);

void setPlaylistLimitCount (int &selectedTrackLimitCode, int *s_playlistActualCntSelCode);

#endif // PLAYLISTFUNCTIONS_H
