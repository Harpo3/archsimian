#ifndef ARCHSIMIAN_H
#define ARCHSIMIAN_H

#include <QStandardPaths>
#include <QMainWindow>
#include <QtWidgets/QToolBar>
#include <QAction>

namespace Ui {
class ArchSimian;
//class QAction;
//class QMenu;
}

class ArchSimian : public QMainWindow
{
    Q_OBJECT

public:
    explicit ArchSimian(QWidget *parent = nullptr);

//    ~ArchSimian();

public slots:

    void on_setlibraryButton_clicked();
    void on_setmmplButton_clicked();
    void on_setmmdbButton_clicked();    
    void on_windowsDriveLtrEdit_textChanged(const QString &arg1);
    void on_addsongsButton_released();
    void on_saveConfigButton_released();

private slots:

    void on_mainQTabWidget_tabBarClicked(int index);
    void on_addtrksspinBox_valueChanged(int s_numTracks);
    void on_repeatFreq1SpinBox_valueChanged(int myvalue);
    void on_weeksradioButton_clicked();
    void on_monthsradioButton_clicked();
    void on_factor3horizontalSlider_valueChanged(int value);
    void on_daysradioButton_clicked();
    void on_yearsradioButton_clicked();
    void on_factor4doubleSpinBox_valueChanged(double argfact4);
    void on_factor5doubleSpinBox_valueChanged(double argfact5);
    void on_factor6doubleSpinBox_valueChanged(double argfact6);
    void on_factor7doubleSpinBox_valueChanged(double argfact7);
    void on_factor8doubleSpinBox_valueChanged(double argfact8);
    void on_InclNewcheckbox_stateChanged(int inclNew);
    void on_albumscheckBox_stateChanged(int inclAlbums);
    void on_minalbumsspinBox_valueChanged(int arg1);
    void on_mintracksspinBox_valueChanged(int arg1);
    void on_mintrackseachspinBox_valueChanged(int arg1);
    void on_actionExport_Playlist_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionSave_Settings_triggered();
    void on_actionAbout_triggered();
    void on_actionOpen_Playlist_triggered();
    void on_actionNew_Playlist_triggered();
    void on_autosavecheckBox_stateChanged(int autosave);
    void on_disablenotecheckBox_stateChanged(int disableNote);
    void on_resetpushButton_released();
    void on_viewplaylistButton_clicked();   
    //void on_playlistLimitSlider_valueChanged(int value);

    void on_actionIterate_tag_triggered();
    void on_actionGet_Last_Played_Dates_triggered();
    void on_actionDate_Conversion_triggered();
    void on_actionSyncPlaylist_triggered();

    void on_actionRemove_mp3s_triggered();

    void on_mmdisabledradioButton_clicked();

    void on_mmenabledradioButton_2_clicked();



    void on_selectAndroidDeviceButton_clicked();

    void on_updateASDBButton_clicked();

    void on_actionUpdateLastPlayed_triggered();

    void on_actionExport_Playlist_to_Linux_triggered();

    void on_syncPlaylistButton_clicked();

    void on_syncthingButton_clicked();

    void on_actionsyncAudaciousLog_triggered();

    void on_enableAudaciousLogButton_clicked();

    void on_enableAIMPOnlyradioButton_clicked();

private:
    Ui::ArchSimian *ui;
    QString m_sSettingsFile;
//    QToolBar *mainToolBar;

public:
    void closeEvent(QCloseEvent *event) override;

private:
    struct SPreferences
    {
        int repeatFreqCode1{};
        int tracksToAdd{};
        QString defaultPlaylist{};
        QString musicLibraryDir{};
        QString mmBackupDBDir{};
        QString mmPlaylistDir{};
        QString s_WindowsDriveLetter{};
        QString s_musiclibshortened{};
        QString s_windowstopfolder{};
        QString s_androidpathname{};
        QString s_syncthingpathname{};
        bool s_includeNewTracks{};
        bool s_includeAlbumVariety{};
        bool s_noAutoSave{};
        bool s_disableNotificationAddTracks{};
        double s_daysTillRepeatCode3{};
        double s_repeatFactorCode4{};
        double s_repeatFactorCode5{};
        double s_repeatFactorCode6{};
        double s_repeatFactorCode7{};
        double s_repeatFactorCode8{};
        int s_minalbums{};
        int s_mintrackseach{};
        int s_mintracks{};
        bool s_mm4disabled{};
        bool s_audaciouslogenabled;
    };

    void loadSettings();
    void saveSettings();

    SPreferences m_prefs;
    };


#endif // ARCHSIMIAN_H
