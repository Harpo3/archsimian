#ifndef DIALOGSETUP_H
#define DIALOGSETUP_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class DialogSetup;
}

class DialogSetup : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetup(QWidget *parent = nullptr);
    ~DialogSetup();
    void closeEvent(QCloseEvent *event);
    void loadSettings();



private slots:
    void on_setlibraryButton1_clicked();

    void on_setmmplButton_clicked();

    void on_setmmdbButton_clicked();

private:
    Ui::DialogSetup *ui;
    QString m_sSettingsFile1;
    struct SPreferences1
    {
        QString musicLibraryDir;
        QString mmBackupDBDir;
        QString mmPlaylistDir;
    };
     SPreferences1 m_prefs1;

     void loadSettings1();
     void saveSettings1();
};

#endif // DIALOGSETUP_H
