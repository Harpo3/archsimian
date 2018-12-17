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

    void on_setlibraryButtonReset_clicked();

    void on_setmmplButtonReset_clicked();

    void on_setmmdbButtonReset_clicked();


private slots:





private:
    Ui::ArchSimian *ui;



private:




};

#endif // ARCHSIMIAN_H
