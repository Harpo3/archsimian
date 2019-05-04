#ifndef PLAYLISTCONTENTDIALOG_H
#define PLAYLISTCONTENTDIALOG_H

#include <QDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>


namespace Ui {
class PlaylistContentDialog;
}

class PlaylistContentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlaylistContentDialog(QWidget *parent = nullptr);
    ~PlaylistContentDialog();


private:
    Ui::PlaylistContentDialog *ui;

};

#endif // PLAYLISTCONTENTDIALOG_H
