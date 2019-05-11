#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>
#include <QString>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

using StringVector = std::vector<std::string>;
using StringVector2D = std::vector<StringVector>;

inline bool doesFileExist (const std::string& name) {
    struct stat buffer{};
    return (stat (name.c_str(), &buffer) == 0);
}

int countBlankChars(std::string input)
{
    int spaces = 0;
    unsigned long totalchars = input.length();
    for (unsigned long i = 0; i< totalchars; ++i){
        if (isspace(input[i]))
            ++spaces;}
        return spaces;
}

StringVector2D readDSV(const std::string& filename)
{
    char separator = '^';
    StringVector2D result;
    std::string row;
    std::string item;
    std::ifstream in(filename);
    while(getline(in,row))
    {
        StringVector R;
        std::stringstream ss(row);
        while (getline(ss,item,separator))R.push_back(item);
        result.push_back(R);
    }
    in.close();
    return result;
}

StringVector2D readCSV(const std::string& filename)
{
    char separator = ',';
    StringVector2D result;
    std::string row;
    std::string item;

    std::ifstream in(filename);
    while(getline(in,row))
    {
        StringVector R;
        std::stringstream ss(row);
        while (getline(ss,item,separator))R.push_back(item);
        result.push_back(R);
    }
    in.close();
    return result;
}

void trim_cruft(std::string& buffer)
{
    static const char cruft[] = "\n\r";
    buffer.erase(buffer.find_last_not_of(cruft) + 1);
}

// Compares two strings and returns bool for match result
bool stringMatch(std::string s1, std::string s2)
{
    bool x{false};
    trim_cruft(s1);
    trim_cruft(s2);
    if(s1 == s2)x = true;
    //else x = true;
    return x;
}

// Looks for str in ifstream and returns a bool for match result
bool matchLineinIfstream(std::ifstream & stream, std::string str) {
    std::string line;
    bool x{false};
    while (getline(stream, line)) {
        x = stringMatch(line,str);
        if (x) break;
    }
    return x;
}

// Function to remove all spaces from a given string
std::string removeSpaces(std::string str)
{
    str.erase(remove(str.begin(), str.end(), ' '), str.end());
    return str;
}

// Function to return a vector as a single DSV string with carat delimiter
std::string getChgdDSVStr(std::vector<std::string> const &input,std::string chgdString) {
    std::ostringstream oschgdString;
    for (const auto & i : input) {
        oschgdString << i << '^';
    }
    chgdString = oschgdString.str();
    return chgdString;
}

// Function to return all vector contents to a single dir path string using a '/' delimiter
std::string getChgdDirStr(std::vector<std::string> const &input, std::string chgdString, const QString &s_musiclibrarydirname) {
    std::ostringstream oschgdString;
    for (unsigned long i = 1; i < input.size(); i++) {
        oschgdString << '/' << input.at(i);
    }
    chgdString = s_musiclibrarydirname.toStdString() + oschgdString.str();
    return chgdString;
}

// Function to read a line of data that may have variable line ending chars
// The characters in the stream are read one-by-one using a std::streambuf.
// That is faster than reading them one-by-one using the std::istream.
// Code that uses streambuf this way must be guarded by a sentry object.
// The sentry object performs various tasks,
// such as thread synchronization and updating the stream state.

std::istream& safeGetline(std::istream& is, std::string& t){
    t.clear();
    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();
    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case std::streambuf::traits_type::eof():
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += static_cast<char>(c);
        }
    }
}

int cstyleStringCount(std::string path){
    std::ifstream ifs(path.c_str());
    if(!ifs) {
        std::cout << "Failed to open the file." << std::endl;
        return EXIT_FAILURE;
    }
    int n = 0;
    std::string t;
    while(!safeGetline(ifs, t).eof())
        ++n;
    //std::cout << "The file contains " << n << " lines." << std::endl;
    return n;
}

void removeAppData(std::string str)
{
    QString appDataPathstr = QDir::homePath() + "/.local/share/archsimian";
    str = appDataPathstr.toStdString() +"/" + str;
    bool existResult;
    existResult = doesFileExist(str);// See inline function at top
    //std::cout <<"removeAppData: Removing: "<< str << std::endl;
    if (existResult == 1) {
        remove (str.c_str());  // remove file referenced by str from the AppData directory
        if(remove( str.c_str() ) != 0 ) {
            std::cout <<"removeAppData: Error deleting file: "<< str << std::endl;
        }
        else
            puts( "removeAppData: File successfully deleted" );
    }
}

std::vector<std::string> split(std::string strToSplit, char delimeter){
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    splittedStrings.reserve(10000);
    while (std::getline(ss, item, delimeter)) {
        splittedStrings.push_back(item);
    }
    return splittedStrings;
}
