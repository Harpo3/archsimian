#ifndef UTILITIES_H
#define UTILITIES_H
#include <QString>
#include <vector>
#include <map>

typedef std::vector<std::string> StringVector;
typedef std::vector<StringVector> StringVector2D;
using StringVector = std::vector<std::string>;
using StringVector2D = std::vector<StringVector>;

int countBlankChars(std::string input);
StringVector2D readDSV(std::string filename);
StringVector2D readCSV(std::string filename);
void trim_cruft(std::string& buffer);
bool stringMatch(std::string s1, std::string s2);
bool matchLineinIfstream(std::ifstream & stream, std::string str);
std::string removeSpaces(std::string str);
std::string getChgdDSVStr(std::vector<std::string> const &input,std::string chgdString);
std::string getChgdDirStr(std::vector<std::string> const &input, std::string chgdString, const QString &s_musiclibrarydirname);
std::istream& safeGetline(std::istream& is, std::string& t);
int cstyleStringCount(std::string path);
void removeAppData(std::string str);
std::vector<std::string> split(std::string strToSplit, char delimeter);


#endif // UTILITIES_H
