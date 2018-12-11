#include <iostream>
#include <fstream>
#include <string>

int getTotalRows(std::string x)
  {
      int count{0};
      std::string line;
      std::ifstream file(x);
      while (getline(file, line))
          count++;
      std::cout << "Numbers of lines in the file : " << count << std::endl;
      return count;
}
