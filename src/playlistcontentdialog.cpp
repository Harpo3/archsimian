#include "playlistcontentdialog.h"
#include "ui_playlistcontentdialog.h"

PlaylistContentDialog::PlaylistContentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlaylistContentDialog)
{
    ui->setupUi(this);
    QString appDataPathstr = QDir::homePath() + "/.local/share/archsimian";
    QFile file(appDataPathstr+"/cleanedplaylist.txt");
    if(!file.open(QIODevice::ReadOnly))
        QMessageBox::information(nullptr,"Current Playlist",file.errorString());
    QTextStream in(&file);
    PlaylistContentDialog::setWindowTitle("ArchSimian - Playlist Contents");
    ui->textBrowser->setText(in.readAll());
}

PlaylistContentDialog::~PlaylistContentDialog()
{
    delete ui;
}
