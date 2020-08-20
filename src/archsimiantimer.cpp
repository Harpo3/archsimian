#include "archsimiantimer.h"

#include <QDebug>

ASTimer::ASTimer()
{
    // create a timer
    astimer = new QTimer(this);

    // setup signal and slot
    connect(astimer, SIGNAL(timeout()),
          this, SLOT(ASTimerSlot()));

    // msec
    astimer->start(1000);
}


void ASTimer::ASTimerSlot()
{
    qDebug() << "Timer...";
}



