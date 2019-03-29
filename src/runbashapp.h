#ifndef RUNBASHAPP_H
#define RUNBASHAPP_H
#include <string>
// Function to execute a bash command and get result
std::string exec(const char* cmd);
template <std::size_t N>

int execvp(const char* file, const char* const (&argv)[N]);
#endif // RUNBASHAPP_H
