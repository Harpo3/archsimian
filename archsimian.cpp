//archsimian.cpp is equivalent to MainWindow.cpp. This and archsimian.h are
// where the UI code resides. The main () function is located at main.cpp

#include "archsimian.h"
#include "ui_archsimian.h"
//#include "userconfig.h"
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
    //*** need to add code to display this (if userconfig was
    //not already set, see next comment):
    ui->setlibrarylabel->setText(tr("Select the base directory of "
                                    "your music library"));
    ui->setmmpllabel->setText(tr("Select the shared Windows directory"
                                 " where you stored the backup playlists from MediaMonkey"));
    ui->setmmdblabel->setText(tr("Select the shared Windows directory"
                                 " where you stored the MediaMonkey database backup file"));
    //Add a variable for isUserConfigFile, then if false: set
    //centralWidget::mainQTabWidget::playlist::enabled   to false, then do this for the
    // other tabs except for Configuration
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
