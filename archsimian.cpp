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

    // Set default state of false for reset buttons
    ui->setlibraryButtonReset->setVisible(false);
    ui->setmmplButtonReset->setVisible(false);
    ui->setmmdbButtonReset->setVisible(false);

    // If user config has been executed, read the config file contents into the variables,
    // then load the existing locations from archsimian.conf to populate the ui labels
    if (configSetupResult == 1) {
        std::string line;
        std::ifstream myFile (Constants::userFileName);
        for (int lineno = 0; getline (myFile,line) && lineno < 6; lineno++)
        {
            if (lineno == 1) // "1" is the second line of the archsimian.conf file
                // Get the location of music library from archsimian.conf, put into QString musiclibrarydirname
            {musiclibrarydirname = QString::fromStdString(line);
                // Load the existing locations to the ui label
                ui->setlibrarylabel->setText(musiclibrarydirname);}
            //dim the current button
            ui->setlibraryButton->setEnabled(false);
            //enable the reset button
            ui->setlibraryButtonReset->setVisible(true);

            if (lineno == 3)
            {mmbackuppldirname = QString::fromStdString(line);
                ui->setmmpllabel->setText(mmbackuppldirname);}
            //dim the current button
            ui->setmmplButton->setEnabled(false);
            //enable the reset button
            ui->setmmplButtonReset->setVisible(true);

            if (lineno == 5)
            {mmbackupdbdirname = QString::fromStdString(line);
                ui->setmmdblabel->setText(mmbackupdbdirname);}
            //dim the current button
            ui->setmmdbButton->setEnabled(false);
            //enable the reset button
            ui->setmmdbButtonReset->setVisible(true);
        }
    }
    // Otherwise, user config has not been set. Load instructions to ui for user to locate and set config
    else {
        ui->setlibrarylabel->setText(tr("Select the base directory of "
                                        "your music library"));
        ui->setlibraryButton->setEnabled(true);
        ui->setlibraryButtonReset->setVisible(false);
        ui->setmmpllabel->setText(tr("Select the shared Windows directory"
                                     " where you stored the backup playlists from MediaMonkey"));
        ui->setmmplButton->setEnabled(true);
        ui->setmmplButtonReset->setVisible(false);
        ui->setmmdblabel->setText(tr("Select the shared Windows directory"
                                     " where you stored the MediaMonkey database backup file"));
        ui->setmmdbButton->setEnabled(true);
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
    //dim the current button
    ui->setlibraryButton->setEnabled(false);
    //enable the reset button
    ui->setlibraryButtonReset->setVisible(true);
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
        //dim the current button
        ui->setmmplButton->setEnabled(false);
        //enable the reset button
        ui->setmmplButtonReset->setVisible(true);
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
    //dim the current button
    ui->setmmdbButton->setEnabled(false);
    //enable the reset button
    ui->setmmdbButtonReset->setVisible(true);
        }
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
