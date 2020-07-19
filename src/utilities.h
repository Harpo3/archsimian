#ifndef UTILITIES_H
#define UTILITIES_H
#include <QString>
#include <vector>
#include <map>

using StringVector = std::vector<std::string>;
using StringVector2D = std::vector<StringVector>;

bool comp(int a, int b);

const std::string emptyString = "";
std::string ExtractString( std::string source, std::string start, std::string end );

int countBlankChars(std::string input);

int countDelimChars(std::string input);

StringVector2D readDSV(const std::string& filename);
StringVector2D readCSV(const std::string& filename);

void trim_cruft(std::string& buffer);

bool stringMatch(std::string s1, std::string s2);

bool matchLineinIfstream(std::ifstream & stream, std::string str);

std::string removeSpaces(std::string str);

std::string getChgdDSVStr(std::vector<std::string> const &input,std::string chgdString);

std::string getChgdDirStr(std::vector<std::string> const &input, std::string chgdString, std::string musiclibshortened);

std::istream& safeGetline(std::istream& is, std::string& t);

int cstyleStringCount(std::string path);

void removeAppData(std::string str);

std::vector<std::string> split(std::string strToSplit, char delimeter);

std::string GetStdoutFromCommand(std::string cmd);

std::string getCurrentDateTime( std::string s );

void Logger( std::string logMsg );

#endif // UTILITIES_H
