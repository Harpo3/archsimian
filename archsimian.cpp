//archsimian.cpp is equivalent to MainWindow.cpp. This and archsimian.h are
// where the UI code resides. The main () function is located at main.cpp

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>
#include <iostream>
#include <fstream>
#include <string>
#include "archsimian.h"
#include "constants.h"
#include "ui_archsimian.h"
#include "userconfig.h"


ArchSimian::ArchSimian(QWidget *parent) :    
    QMainWindow(parent),
    ui(new Ui::ArchSimian)
{
    //QString musiclibrarydirnamegui;
    ui->setupUi(this);
    // User configuration: set default state to "false" for user config reset buttons
    ui->setlibraryButtonReset->setVisible(false);
    ui->setmmplButtonReset->setVisible(false);
    ui->setmmdbButtonReset->setVisible(false);

    //Check whether the configuration file has any data in it
    std::streampos size;
    char * memblock;
    std:: ifstream file ("archsimian.conf", std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
      size = file.tellg();
      memblock = new char [size];
      file.seekg (0, std::ios::beg);
      file.read (memblock, size);
      file.close();
      delete[] memblock;
    }
    else std::cout << "Unable to open configuration file";

    //If user config has already been set, populate the ui labels accordingly 

    if (size != 0)
    {
                std::string musiclibrarydirname = userconfig::getConfigEntry(1); // archsimian.conf: 1=music lib, 3=playlist dir, 5=mm.db dir
                ui->setlibrarylabel->setText(QString::fromStdString(musiclibrarydirname));
                //dim the setlibraryButton button
                ui->setlibraryButton->setEnabled(false);
                //enable the reset button
                ui->setlibraryButtonReset->setVisible(true);
                std::string mmbackuppldirname = userconfig::getConfigEntry(3); // archsimian.conf: 1=music lib, 3=playlist dir, 5=mm.db dir
                ui->setmmpllabel->setText(QString::fromStdString(mmbackuppldirname));
                //dim the setmmplButton button
                ui->setmmplButton->setEnabled(false);
                //enable the reset button
                ui->setmmplButtonReset->setVisible(true);
                std::string mmbackupdbdirname = userconfig::getConfigEntry(5); // archsimian.conf: 1=music lib, 3=playlist dir, 5=mm.db dir
                ui->setmmdblabel->setText(QString::fromStdString(mmbackupdbdirname));
                //dim the setmmdbButton button
                ui->setmmdbButton->setEnabled(false);
                //enable the reset button
                ui->setmmdbButtonReset->setVisible(true);}

    else {      // Otherwise, user config has not been set. Load instructions to ui for user to locate and set config
                // Initially, only the first of three config buttons is activated. Build the file in sequence.
        ui->setlibrarylabel->setText(tr("Select the base directory of "
                                        "your music library"));
        ui->setlibraryButton->setEnabled(true);
        ui->setlibraryButtonReset->setVisible(false);
        ui->setmmpllabel->setText(tr("Select the shared Windows directory"
                                     " where you stored the backup playlists from MediaMonkey"));
        ui->setmmplButton->setEnabled(false);
        ui->setmmplButtonReset->setVisible(false);
        ui->setmmdblabel->setText(tr("Select the shared Windows directory"
                                     " where you stored the MediaMonkey database backup file"));
        ui->setmmdbButton->setEnabled(false);
        ui->setmmdbButtonReset->setVisible(false);
    }
}
void ArchSimian::on_addsongsButton_clicked(){

}

void ArchSimian::on_importplaylistButton_clicked(){

}

void ArchSimian::on_exportplaylistButton_clicked(){

}

void ArchSimian::on_setlibraryButton_clicked(){
    QFileDialog setlibraryButton;
//    if( !setlibraryButton.exec() )
//    {
        // The user pressed the cancel button so handle this accordingly
//        return;
//    }
//    else {
        setlibraryButton.setFileMode(QFileDialog::Directory);
        setlibraryButton.setOption(QFileDialog::ShowDirsOnly);
        const QString musiclibrarydirname=QFileDialog::getExistingDirectory(
                    this,
                    tr("Select Shared Music Library Directory"),
                    "/"
                    );        
        ui->setlibrarylabel->setText(QString(musiclibrarydirname));

        // Write description note and directory configuration to archsimian.conf
        std::ofstream userconfig(Constants::userFileName);
        std::string str("# Location of music library");
        userconfig << str << std::endl;  // Write to line 1, archsimian.conf
        str = musiclibrarydirname.toStdString();
        userconfig << str << std::endl;  // Write to line 2, archsimian.conf
        userconfig.close();

        // dim the setlibraryButton button
        ui->setlibraryButton->setEnabled(false);
        //enable the reset button
        ui->setlibraryButtonReset->setVisible(true);
        // Activate the second of three config buttons
        ui->setmmplButton->setEnabled(true);
 //      }
}

void ArchSimian::on_setmmplButton_clicked(){  
        QFileDialog setmmpldialog;
 //       if( !setmmpldialog.exec() )
//        {
            // The user pressed the cancel button so handle this accordingly
 //           return;
 //       }
 //       else {
            setmmpldialog.setFileMode(QFileDialog::Directory);
            setmmpldialog.setOption(QFileDialog::ShowDirsOnly);
            const QString mmbackuppldirname=QFileDialog::getExistingDirectory(
                        this,
                        tr("Select MediaMonkey Playlist Backup Directory"),
                        "/"
                        );            
            ui->setmmpllabel->setText(QString(mmbackuppldirname));

            // Write description note and directory configuration to archsimian.conf
            std::ofstream userconfig(Constants::userFileName, std::ios::app);
            std::string str("# Location of MediaMonkey Playlist Backup Directory");
            userconfig << str << std::endl;  // Write to line 3, archsimian.conf
            str = mmbackuppldirname.toStdString();
            userconfig << str << std::endl; // Write to line 4, archsimian.conf
            userconfig.close();

            //dim the setmmplButton button
            ui->setmmplButton->setEnabled(false);
            //enable the reset button
            ui->setmmplButtonReset->setVisible(true);
            // Activate the last of three config buttons
            ui->setmmdbButton->setEnabled(true);
 //       }
}

void ArchSimian::on_setmmdbButton_clicked(){
        QFileDialog setmmdbdialog;
 //       if( !setmmdbdialog.exec() )
 //       {
            // The user pressed the cancel button so handle this accordingly
 //           return;
//        }
 //       else {
        setmmdbdialog.setFileMode(QFileDialog::Directory);
        setmmdbdialog.setOption(QFileDialog::ShowDirsOnly);
        const QString mmbackupdbdirname=QFileDialog::getExistingDirectory(
            this,
            tr("Select MediaMonkey Database (MM.DB) Backup Directory"),
            "/"
            );
        ui->setmmdblabel->setText(QString(mmbackupdbdirname));

        // Write description note and directory configuration to archsimian.conf
        std::ofstream userconfig(Constants::userFileName, std::ios::app);
        std::string str("# Location of MediaMonkey Database Backup Directory");
        userconfig << str << std::endl;  // Write to line 5, archsimian.conf
        str = mmbackupdbdirname.toStdString();
        userconfig << str << std::endl; // Write to line 6, archsimian.conf
        userconfig.close();

        //dim the setmmdbButton button
        ui->setmmdbButton->setEnabled(false);
        //enable the reset button
        ui->setmmdbButtonReset->setVisible(true);
//        }
}

void ArchSimian::on_setlibraryButtonReset_clicked()
{
    //dim the reset button

    //enable the current button
}

void ArchSimian::on_setmmplButtonReset_clicked()
{
    //dim the reset button

    //enable the current button
}

void ArchSimian::on_setmmdbButtonReset_clicked()
{
    //dim the reset button

    //enable the current button
}
