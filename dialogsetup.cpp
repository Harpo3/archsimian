#include <QFileDialog>
#include <QSettings>
#include <QCloseEvent>
#include "dialogsetup.h"
#include "ui_dialogsetup.h"
#include "archsimian.h"

DialogSetup::DialogSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetup)
{
    ui->setupUi(this);
    m_sSettingsFile1 = QApplication::applicationDirPath().left(1) + ":/archsimian.conf";
}

DialogSetup::~DialogSetup()
{
    delete ui;
}


void DialogSetup::loadSettings1()
{
    QSettings settings;

    m_prefs1.musicLibraryDir = settings.value("musicLibraryDir", "").toString();
    m_prefs1.mmBackupDBDir = settings.value("mmBackupDBDir", "").toString();
    m_prefs1.mmPlaylistDir = settings.value("mmPlaylistDir", "").toString();
}

void DialogSetup::saveSettings1()
{
    QSettings settings1;
    settings1.setValue("musicLibraryDir",m_prefs1.musicLibraryDir);
    settings1.setValue("mmBackupDBDir",m_prefs1.mmBackupDBDir);
    settings1.setValue("mmPlaylistDir",m_prefs1.mmPlaylistDir);
}
void DialogSetup::closeEvent(QCloseEvent *event)
{
    saveSettings1();
    event->accept();
}

void DialogSetup::on_setlibraryButton1_clicked()
{
    QFileDialog setlibraryButton1;
    //s_mmBackupDBDir = m_prefs.mmBackupDBDir;
    QString s_musiclibrarydirname ;
    //s_defaultPlaylist = m_prefs.defaultPlaylist;
    //mmPlaylistDir = m_prefs.mmPlaylistDir;
    //    if( !setlibraryButton.exec() )
    //    {
    // The user pressed the cancel button so handle this accordingly
    //        return;
    //    }
    //    else {
    setlibraryButton1.setFileMode(QFileDialog::Directory);
    setlibraryButton1.setOption(QFileDialog::ShowDirsOnly);
    s_musiclibrarydirname= QFileDialog::getExistingDirectory(
                this,
                tr("Select Shared Music Library Directory"),
                "/"
                );
    ui->setlibrarylabel->setText(QString(s_musiclibrarydirname));

    // Write description note and directory configuration to archsimian.conf
    m_prefs1.musicLibraryDir = s_musiclibrarydirname;

    //ui->setmmplButton->setEnabled(true);
}

void DialogSetup::on_setmmplButton_clicked()
{
    QFileDialog setmmpldialog;
    QString mmbackuppldirname;
    //       if( !setmmpldialog.exec() )
    //        {
    // The user pressed the cancel button so handle this accordingly
    //           return;
    //       }
    //       else {
    setmmpldialog.setFileMode(QFileDialog::Directory);
    setmmpldialog.setOption(QFileDialog::ShowDirsOnly);
    mmbackuppldirname=QFileDialog::getExistingDirectory(
                this,
                tr("Select MediaMonkey Playlist Backup Directory"),
                "/"
                );
    ui->setmmpllabel->setText(mmbackuppldirname);
    m_prefs1.mmPlaylistDir = mmbackuppldirname;
}

void DialogSetup::on_setmmdbButton_clicked()
{
    QFileDialog setmmdbdialog;
    QString mmbackupdbdirname;
      //       if( !setmmdbdialog.exec() )
      //       {
      // The user pressed the cancel button so handle this accordingly
      //           return;
      //        }
      //       else {
      setmmdbdialog.setFileMode(QFileDialog::Directory);
      setmmdbdialog.setOption(QFileDialog::ShowDirsOnly);
      mmbackupdbdirname=QFileDialog::getExistingDirectory(
                  this,
                  tr("Select MediaMonkey Database (MM.DB) Backup Directory"),
                  "/"
                  );
      ui->setmmdblabel->setText(mmbackupdbdirname);
      m_prefs1.mmBackupDBDir = mmbackupdbdirname;
}
