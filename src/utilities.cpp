#include <QApplication>
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


bool comp(int a, int b)
{
    return (a < b);
}

const std::string emptyString = "";
std::string ExtractString( std::string source, std::string start, std::string end )
{
     std::size_t startIndex = source.find( start );
     // If the starting delimiter is not found on the string
     // stop the process, you're done!
     if( startIndex == std::string::npos )
     {
        return emptyString;
     }
     // Adding the length of the delimiter to our starting index
     // this will move us to the beginning of our sub-string.
     startIndex += start.length();
     // Looking for the end delimiter
     std::string::size_type endIndex = source.find( end, startIndex );
     // Returning the substring between the start index and
     // the end index. If the endindex is invalid then the
     // returned value is empty string.
     return source.substr( startIndex, endIndex - startIndex );
}

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

int countDelimChars(std::string input)
{
    int delims = 0;
    unsigned long totalchars = input.length();
    for (unsigned long i = 0; i< totalchars; ++i){
        if (input[i] == '\\')
            ++delims;}
        return delims;
}

int positionOfXthDelimChar(std::string input, int xthdelim)
{
    int delims = 0;
    unsigned long posnumber = 0;
    unsigned long totalchars = input.length();
    for (unsigned long i = 0; i< totalchars; ++i){
        if (input[i] == '/')
            ++delims;
        if (delims == xthdelim){
            // Set position number
            posnumber = i;
        }
    }
        return int(posnumber);
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
std::string getChgdDirStr(std::vector<std::string> const &input, std::string chgdString, std::string musiclibshortened) {
            std::ostringstream oschgdString;
            for (unsigned long i = 1; i < input.size(); i++) {
                oschgdString << '/' << input.at(i);
            }
            chgdString = musiclibshortened + oschgdString.str(); // changed here too
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

std::string GetStdoutFromCommand(std::string cmd) {

std::string data;
FILE * stream;
const int max_buffer = 256;
char buffer[max_buffer];
cmd.append(" 2>&1");
stream = popen(cmd.c_str(), "r");
if (stream) {
  while (!feof(stream))
    if (fgets(buffer, max_buffer, stream) != nullptr) data.append(buffer);
      pclose(stream);
  }
return data;
}

std::string getCurrentDateTime( std::string s ){
    time_t now = time(nullptr);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    if(s=="now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if(s=="date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return std::string(buf);
}

void Logger( std::string logMsg ){

    QString appDataPathstr = QDir::homePath() + "/.local/share/" + QApplication::applicationName();
    std::string logd = appDataPathstr.toStdString()+"/diagnosticslog.txt";
    std::string filePath = logd;
    std::string now = getCurrentDateTime("now");
    std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app );
    ofs << now << '\t' << logMsg << '\n';
    ofs.close();
}
