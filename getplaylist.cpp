#include <sstream>
#include "userconfig.h"

void getPlaylist(){   //  Purpose is to remove the m3u headers lines, leaving just the file path
    static std::string s_selectedplaylist = userconfig::getConfigEntry(7);//1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
    static std::string s_musiclibrarydirname = userconfig::getConfigEntry(1);
    std::string playlistFile = s_selectedplaylist;
    std::ifstream readFile(playlistFile);
    std::ofstream outf("cleanedplaylist.txt");
    if (!readFile.is_open())
        {
            std::cout << "getPlaylist: The readFile did not open. Did you manually export the MediaMonkey playlist?";
            std::exit(EXIT_FAILURE);
        }
    std::string line;
    //std::getline(readFile, line); //skip first line
    while (std::getline(readFile, line))
    {
        std::istringstream iss(line); //start with first line
        // Fix the dir string: remove colon, insert library user path, and correct the dir symbol from \ to /
        std::string str = line;
        str.replace(str.begin(),str.begin()+2,"");
        line = str;
        std::string str2 ("\\");
        std::size_t found = line.find(str2);
        if (found!=std::string::npos)
        {// colon is one char before the first dir symbol
            line.replace(line.find(str2),str2.length(),s_musiclibrarydirname + "/");
            found=line.find("second dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"/");
            found=line.find("third dir symbol",found+1,1);
            line.replace(line.find(str2),str2.length(),"/");
        }
        outf << line; // DO NOT ADD endl here
    }
    readFile.close();
    outf.close();
}

bool getPlaylistVect(std::string fileName, std::vector<std::string> & plStrings)
{
    // Open the File
    std::ifstream in(fileName.c_str());
    // Check if object is valid
    if(!in)
    {
        std::cerr << "getPlaylistVect: Cannot open the File : "<<fileName<<std::endl;
        return false;
    }
    std::string str;
    // Read the next line from File untill it reaches the end.
    while (std::getline(in, str, '\r'))
    {
        // Line contains string of length > 0 then save it in vector
        if(str.size() > 0)
            plStrings.push_back(str);
    }
    in.close();     //Close The File
    return true;
}