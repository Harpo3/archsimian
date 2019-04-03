#ifndef BUILDALBUMEXCLLIBRARY_H
#define BUILDALBUMEXCLLIBRARY_H
#include <vector>
#include <map>
template <typename T>
void findDuplicates(std::vector<T> & vecOfElements, std::map<T, int> & countMap);
void buildAlbumExclLibrary(const int &s_minalbums, const int &s_mintrackseach, const int &s_mintracks);
#endif // BUILDALBUMEXCLLIBRARY_H
