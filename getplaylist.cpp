#include <sstream>
#include "userconfig.h"
#include <iostream>
#include <fstream>
#include <vector>

std::istream& safeGetline(std::istream& is, std::string& t){
    t.clear();
    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.
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
            t += (char)c;
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
    std::cout << "The file contains " << n << " lines." << std::endl;
    return n;
}

void getPlaylist(){   //  Purpose is to remove the m3u headers lines, leaving just the file path
    static std::string s_selectedplaylist = userconfig::getConfigEntry(7);//1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
    static std::string s_musiclibrarydirname = userconfig::getConfigEntry(1);
    std::string playlistFile = s_selectedplaylist;
    std::ifstream readFile(playlistFile);
    std::ofstream outf("cleanedplaylist.txt");
    if (!readFile.is_open()) {
        std::cout << "getPlaylist: The readFile did not open. Did you manually export the MediaMonkey playlist?";
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    //std::getline(readFile, line); //skip first line
    while (std::getline(readFile, line)){
        std::istringstream iss(line); //start with first line
        // Fix the dir string: remove colon, insert library user path, and correct the dir symbol from \ to /
        std::string str = line;
        str.replace(str.begin(),str.begin()+2,"");
        line = str;
        std::string str2 ("\\");
        std::size_t found = line.find(str2);
        if (found!=std::string::npos) {// colon is one char before the first dir symbol
            line.replace(line.find(str2),str2.length(),s_musiclibrarydirname + "/");
            found=line.find("second dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"/");
            found=line.find("third dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"/");
        }
        outf << line << '\n'; // DO NOT ADD endl here
    }
    readFile.close();
    outf.close();
}
