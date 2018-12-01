#include "archsimian.h"
#include "dependents.h"
#include "userconfig.h"
#include "runbashapp.h"
#include "lastplayeddays.h"
#include "constants.h"
#include <QApplication>
#include <QFileInfo>
#include <iostream>
#include <fstream>
#include <string>
#include <QDir>

int main(int argc, char *argv[]){

    //Basic info: argc (argument count) and argv (argument vector) are
    //how command line arguments are passed to main() in C++.
    //argc will be the number of strings pointed to by argv. This will
    //(in practice) be 1 plus the number of arguments, as virtually all
    //implementations will prepend the name of the program to the array.

    // Sample code to call function for computing days since last played
    // It uses a SQL time value example from MM4.
    double mm4LastPlayedDays{43230.657113};
    mm4LastPlayedDays = getLastPlayedDays(mm4LastPlayedDays);
    std::cout << "\nThis is a placeholder in main for some sample code to "
                 "call the getLastPlayedDays function for computing days \nsince last played. "
                 "Output is a double type variable and its result is: "
              << mm4LastPlayedDays << ".\n";

    // Sample code to call the isAppRunning function to check whether an app is running
    // using its pid description, in this example, audacious.
    bool myTestBool;
    myTestBool = isAppRunning("audacious");
    std::cout << "\nThis is a placeholder in main for some sample code to call the "
                "isAppRunning function with Audacious, \nto check whether the app is running using its pid "
                "description. Output is a bool type \nvariable and its result is: "
             << myTestBool << ".\n";




    //Basic info: Identifies the main GUI app (QApplication) as mainapp
    QApplication mainapp(argc, argv);

    // This launches the user interface (UI)
    ArchSimian guiWindow;
    guiWindow.show();

    // This closes the files opened in the sample section above
    return mainapp.exec();
}







// moved (old):
//QString appendPath(const QString& path1, const QString& path2)
//{
//    return QDir::cleanPath(path1 + QDir::separator() + path2);
//}
// This function determines the user's HOME path, which will be used to
// create and access configuration files

//moved old:
//   ofstream my_output_file;
//   localConfigFilePath.open(localConfigFilePath);
 //  my_output_file.open(localConfigFilePath);
//   if (my_input_file.fail() )
//     {
   //       // I need to recreate the file
//       std::cout << "The file is not there. Need to create one.";
 //     }
//      my_input_file.close();
//      my_output_file.close();

//   if (my_output_file.is_open())
//      {
//    my_output_file << "A test.";
//    my_output_file.close();
//
 //     }
//   return myConfig;
//   }

//moved (old):
//std::ifstream my_output_file;
//my_output_file.open(fullString);
//std::ostream >> "Testing 123";
//bool isopening;
//std::ofstream ifs(fullString);
//ifs.open(fullString);
//fs << "writing this to a file.\n";
//ifs.close();

// char* result; result = calloc(strlen(myStringPath)+strlen(two)+1, sizeof(char)); and THEN the strcpy+strcat?
 //  char result[75];
//   char* (strlen(char* myStringPath) + strlen(char* userFileName[17])+1, sizeof(char));

//   strcpy(result, userFileName);
  // std::cout << "\nThis is the first placeholder in main for some sample code to concatenate the file path,"
//                "\n with the filename. Output is a string type \nvariable and its result is: "
//             << result << ".\n\n";
//  strcat(result, myStringPath);
 //  std::cout << "\nThis is the second placeholder in main for some sample code to concatenate the file path,"
//                "\n with the filename. Output is a string type \nvariable and its result is: "
 //            << result << ".\n\n";

