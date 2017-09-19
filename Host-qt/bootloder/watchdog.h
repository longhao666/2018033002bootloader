#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <QObject>

typedef  void (*dog_barking_fun)(void);       //

class watchDog : public QObject
{
    Q_OBJECT

public:
    watchDog(int interval, dog_barking_fun barking);
    ~watchDog();
    void feed_dog(void);

private:
    dog_barking_fun barking;
    int dog;
    int timeout;

    void dog_barking();
protected:
    void timerEvent(QTimerEvent *event);
};

#endif // WATCHDOG_H
