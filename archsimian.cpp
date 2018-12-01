//archsimian.cpp is equivalent to MainWindow.cpp. This and archsimian.h are
// where the UI code resides. The main () function is located at main.cpp

#include "archsimian.h"
#include "constants.h"
#include "ui_archsimian.h"
#include "userconfig.h"
#include<QDir>
#include<QFileDialog>
#include<QMessageBox>
#include <QTextStream>
#include <QFileInfo>
#include <iostream>
#include <fstream>
#include <string>

ArchSimian::ArchSimian(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ArchSimian)
{
    ui->setupUi(this);


    // Call the isConfigSetup function to set a bool used for prompting user
    // to setup the program settings before program operation.
    int configSetupResult;
    configSetupResult = isConfigSetup();
    std::cout << "\nThe bool configSetupResult tests whether the user configuration exists (1) or not (0)."
                 " The current result is: " << configSetupResult << ".\n";

    // Declare user configuration variables
    QString musiclibrarydirname;
    QString mmbackuppldirname;
    QString mmbackupdbdirname;

    // If user config has been executed, read the config file contes into the variables
    // First the location of music library into QString musiclibrarydirname
    if (configSetupResult == 1) {
        std::string line;
        std::ifstream myFile (Constants::userFileName);
        for (int lineno = 0; getline (myFile,line) && lineno < 6; lineno++)
        {
              if (lineno == 1)
              {musiclibrarydirname = QString::fromStdString(line);
              ui->setlibrarylabel->setText(musiclibrarydirname);}
              //std::cout << "\nLocation of music library placed into QString variable "
             //              "musiclibrarydirname: " << musiclibrarydirname.toStdString() << ".\n";}
              if (lineno == 3)
              {mmbackuppldirname = QString::fromStdString(line);
              ui->setmmpllabel->setText(mmbackuppldirname);}
                 // std::cout << "\nLocation of MediaMonkey Playlist Backup Directory placed into QString variable "
                            //   "mmbackuppldirname: " << mmbackuppldirname.toStdString() << ".\n";}
              if (lineno == 5)
              {mmbackupdbdirname = QString::fromStdString(line);
               //If userconfig was already set, load the existing locations to the ui labels:
              ui->setmmdblabel->setText(mmbackupdbdirname);}
                 // std::cout << "\nLocation of MediaMonkey Database Backup Directory placed into QString variable "
                              // "mmbackupdbdirname: " << mmbackupdbdirname.toStdString() << "\n.";}
        }
    }
    // Otherwise, user config has not been set. Load instructions to ui for user to locate and set config
    else {
        ui->setlibrarylabel->setText(tr("Select the base directory of "
                                        "your music library"));
        ui->setmmpllabel->setText(tr("Select the shared Windows directory"
                                     " where you stored the backup playlists from MediaMonkey"));
        ui->setmmdblabel->setText(tr("Select the shared Windows directory"
                                       " where you stored the MediaMonkey database backup file"));
        }
}
void ArchSimian::on_addsongsButton_clicked(){

}

void ArchSimian::on_importplaylistButton_clicked(){

}

void ArchSimian::on_exportplaylistButton_clicked(){

}

void ArchSimian::on_setlibraryButton_clicked(){
{
    QFileDialog dialog;
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly);
        QString musiclibrarydirname=QFileDialog::getExistingDirectory(
                    this,
                    tr("Select Shared Music Library Directory"),
                    "/"
                    );
        {
    ui->setlibrarylabel->setText(QString(musiclibrarydirname));
    // Open ifstream archsimian.conf
    //Write "# Location of music library" to line 1, archsimian.conf
    // Write musiclibrarydirname to line 2
    // Close file
       }
}
}

void ArchSimian::on_setmmplButton_clicked(){
    {
        QFileDialog setmmpldialog;
            setmmpldialog.setFileMode(QFileDialog::Directory);
            setmmpldialog.setOption(QFileDialog::ShowDirsOnly);
            QString mmbackuppldirname=QFileDialog::getExistingDirectory(
                        this,
                        tr("Select MediaMonkey Playlist Backup Directory"),
                        "/"
                        );
            {
        ui->setmmpllabel->setText(QString(mmbackuppldirname));
        // Open ifstream archsimian.conf
        //Write "# Location of MediaMonkey Playlist Backup Directory" to line 3, archsimian.conf
        // Write mmbackuppldirname to line 4
        // Close file
            }
    }
}

void ArchSimian::on_setmmdbButton_clicked(){
        QFileDialog setmmdbdialog;
        setmmdbdialog.setFileMode(QFileDialog::Directory);
        setmmdbdialog.setOption(QFileDialog::ShowDirsOnly);
        QString mmbackupdbdirname=QFileDialog::getExistingDirectory(
            this,
            tr("Select MediaMonkey Database (MM.DB) Backup Directory"),
            "/"
            );
        {

    ui->setmmdblabel->setText(QString(mmbackupdbdirname));
    // Open ifstream archsimian.conf
    //Write "# Location of MediaMonkey Database Backup Directory" to line 5, archsimian.conf
    // Write mmbackuppldirname to line 6
    // Close file
        }
}
