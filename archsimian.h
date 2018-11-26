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

    void on_importplaylistButton_clicked();

    void on_exportplaylistButton_clicked();


private:
    Ui::ArchSimian *ui;
};

#endif // ARCHSIMIAN_H
