#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdio.h>

void fixsort()
{
    // Now reopen, then use a vector to remove duplicates again and resort
    std::vector<std::string>fsexclvec;
    fsexclvec.reserve(10000);
    std::ifstream finalexclist;
    std::string str2; // store the string for artistexcludes.txt

    finalexclist.open ("artistexcludes.txt");
    if (finalexclist.is_open()) {finalexclist.close();}
    else {std::cout << "fixsort: Error opening artistexcludes2.txt file." << std::endl;}
    std::string finallist = "artistexcludes.txt"; // now we can use it as input file
    std::ifstream finalTable(finallist);
    if (!finalTable.is_open())
    {
        std::cout << "fixsort: Error opening finalTable." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::ofstream artistExcList2("finalartistexcludes.txt"); // output file for writing final exclude list

    while (std::getline(finalTable, str2)) {fsexclvec.push_back(str2);}
    finalTable.close();
    std::vector<std::string>::iterator ip;
    std::sort (fsexclvec.begin(), fsexclvec.end());
    size_t myvecsz = fsexclvec.size();
    ip = std::unique(fsexclvec.begin(), fsexclvec.begin() + myvecsz); // 500 here is max number of unique artists - need to set variable
    fsexclvec.resize(std::distance(fsexclvec.begin(), ip));
    for (ip = fsexclvec.begin(); ip != fsexclvec.end(); ++ip) {
        artistExcList2 << *ip << "\n";
    }
    artistExcList2.close();
    fsexclvec.shrink_to_fit();
}
