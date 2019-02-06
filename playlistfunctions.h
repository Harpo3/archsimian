#ifndef PLAYLISTFUNCTIONS_H
#define PLAYLISTFUNCTIONS_H
void getExcludedArtists(long *_shistCount, int *_splaylistSize);
int ratingCodeSelected(double *_sratingRatio3, double *_sratingRatio4, double *_sratingRatio5,
                       double *_sratingRatio6, double *_sratingRatio7, double *_sratingRatio8, int *_srCode1TotTrackQty, int *_srepeatFreqForCode1, int *_repeatFreqCode1);
void selectTrack(int *_sratingNextTrack);

#endif // PLAYLISTFUNCTIONS_H
