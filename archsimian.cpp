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

static std::string s_musiclibrarydirname;
static std::string s_mmbackuppldirname;
static std::string s_mmbackupdbdirname;

ArchSimian::ArchSimian(QWidget *parent) :    
    QMainWindow(parent),
    ui(new Ui::ArchSimian)
{
    // User configuration: set default state to "false" for user config reset buttons
    ui->setupUi(this);   
    ui->setlibraryButtonReset->setVisible(false);
    ui->setmmplButtonReset->setVisible(false);
    ui->setmmdbButtonReset->setVisible(false);

    //Check whether the configuration file currently has any data in it
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

    //If configuration has already been set, populate the ui labels accordingly
    if (size != 0)
    {   // getConfigEntry: 1=musiclib dir, 3=playlist dir, 5=mm.db dir 7=playlist filepath
        std::string s_musiclibrarydirname = userconfig::getConfigEntry(1);
        ui->setlibrarylabel->setText(QString::fromStdString(s_musiclibrarydirname));
        //dim the setlibraryButton button
        ui->setlibraryButton->setEnabled(false);
        //enable the reset button
        ui->setlibraryButtonReset->setVisible(true);
        s_mmbackuppldirname = userconfig::getConfigEntry(3);
        ui->setmmpllabel->setText(QString::fromStdString(s_mmbackuppldirname));
        //dim the setmmplButton button
        ui->setmmplButton->setEnabled(false);
        //enable the reset button
        ui->setmmplButtonReset->setVisible(true);
        std::string s_mmbackupdbdirname = userconfig::getConfigEntry(5);
        ui->setmmdblabel->setText(QString::fromStdString(s_mmbackupdbdirname));
        std::string selectedplaylist = userconfig::getConfigEntry(7);
        ui->setgetplaylistLabel->setText("Selected: " + QString::fromStdString(selectedplaylist));
        //dim the setmmdbButton button
        ui->setmmdbButton->setEnabled(false);
        //enable the reset button
        ui->setmmdbButtonReset->setVisible(true);}

    else {  // Otherwise, configuration has not been set. Load instructions for user to locate and set config
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
        ui->setgetplaylistLabel->setText(tr("Select playlist for adding tracks"));
    }
}

void ArchSimian::on_addsongsButton_clicked(){

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
    const QString s_musiclibrarydirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select Shared Music Library Directory"),
                "/"
                );
    ui->setlibrarylabel->setText(QString(s_musiclibrarydirname));
    // Write description note and directory configuration to archsimian.conf
    std::ofstream userconfig(Constants::userFileName);
    std::string str("# Location of music library");
    userconfig << str << "\n";  // Write to line 1, archsimian.conf
    str = s_musiclibrarydirname.toStdString();
    userconfig << str << "\n";  // Write to line 2, archsimian.conf
    userconfig.close();
    // dim the setlibraryButton button
    ui->setlibraryButton->setEnabled(false);
    // Enable the reset button
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
    const QString s_mmbackuppldirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select MediaMonkey Playlist Backup Directory"),
                "/"
                );
    ui->setmmpllabel->setText(QString(s_mmbackuppldirname));
    // Write description note and directory configuration to archsimian.conf
    std::ofstream userconfig(Constants::userFileName, std::ios::app);
    std::string str("# Location of MediaMonkey Playlist Backup Directory");
    userconfig << str << "\n";  // Write to line 3, archsimian.conf
    str = s_mmbackuppldirname.toStdString();
    userconfig << str << "\n"; // Write to line 4, archsimian.conf
    userconfig.close();
    //dim the setmmplButton button
    ui->setmmplButton->setEnabled(false);
    //enable the reset button
    ui->setmmplButtonReset->setVisible(true);
    // Activate the last of three config buttons
    ui->setmmdbButton->setEnabled(true);
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
    const QString s_mmbackupdbdirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select MediaMonkey Database (MM.DB) Backup Directory"),
                "/"
                );
    ui->setmmdblabel->setText(QString(s_mmbackupdbdirname));
    // Write description note and directory configuration to archsimian.conf
    std::ofstream userconfig(Constants::userFileName, std::ios::app);
    std::string str("# Location of MediaMonkey Database Backup Directory");
    userconfig << str << "\n";  // Write to line 5, archsimian.conf
    str = s_mmbackupdbdirname.toStdString();
    userconfig << str << "\n"; // Write to line 6, archsimian.conf
    userconfig.close();
    //dim the setmmdbButton button
    ui->setmmdbButton->setEnabled(false);
    //enable the reset button
    ui->setmmdbButtonReset->setVisible(true);
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

// User selects playlist from configured directory for 'backup playlists'
void ArchSimian::on_getplaylistButton_clicked()
{
    QFileDialog setgetplaylistdialog;
    QString selectedplaylist = QFileDialog::getOpenFileName (
                this,
                "Select playlist for which you will add tracks",
                QString::fromStdString(userconfig::getConfigEntry(3)),//default dir for playlists
                "playlists(.m3u) (*.m3u)");
    ui->setgetplaylistLabel->setText("Selected: " + QString(selectedplaylist));
    // Write description note and playlist name to archsimian.conf
    std::ofstream userconfig(Constants::userFileName, std::ios::app);
    std::string str("# Name of default playlist");
    userconfig << str << "\n";  // Write to line 7, archsimian.conf
    str = selectedplaylist.toStdString();
    userconfig << str << "\n"; // Write to line 8, archsimian.conf
    userconfig.close();
}
