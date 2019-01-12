//#include <cstdio>
//#include <iostream>
#include <memory>
//#include <stdexcept>
//#include <string>
//#include <array>
//#include <sstream>
//#include <unistd.h>

// Function to execute a bash command and get result

std::string execvp(const char* cmd)
{
    std::array<char, 256> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 256, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

