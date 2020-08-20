#ifndef ARCHSIMIANTIMER_H
#define ARCHSIMIANTIMER_H
#include <QTimer>

class ASTimer : public QObject
{
    Q_OBJECT
public:
    ASTimer();
    QTimer *astimer;

public slots:
    void ASTimerSlot();

};
#endif // ARCHSIMIANTIMER_H
