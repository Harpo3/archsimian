#ifndef ARCHSIMIAN_H
#define ARCHSIMIAN_H

#include <QMainWindow>

namespace Ui {
class ArchSimian;
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

    void on_addsongsButton_clicked();

    void on_getplaylistButton_clicked();

    void on_exportplaylistButton_clicked();




private slots:

    void on_mainQTabWidget_tabBarClicked(int index);

//    void on_pushButton_clicked();

//    void on_refreshdbButton_clicked();

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


private:
    Ui::ArchSimian *ui;
    QString m_sSettingsFile;

public:
    void closeEvent(QCloseEvent *event);

private:
    struct SPreferences
    {
        int repeatFreqCode1;
        int tracksToAdd;
        QString defaultPlaylist;
        QString musicLibraryDir;
        QString mmBackupDBDir;
        QString mmPlaylistDir;
        double s_daysTillRepeatCode3;
        double s_repeatFactorCode4;
        double s_repeatFactorCode5;
        double s_repeatFactorCode6;
        double s_repeatFactorCode7;
        double s_repeatFactorCode8;

    };

    void loadSettings();
    void saveSettings();

    SPreferences m_prefs;



};

#endif // ARCHSIMIAN_H
