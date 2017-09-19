#include "watchdog.h"
#include <QTimerEvent>

watchDog :: watchDog(int interval, dog_barking_fun barking){
    timeout = interval;
    this->barking = barking;
    dog = startTimer(timeout);
}

watchDog :: ~watchDog(){
    if (dog != 0){
        killTimer(dog);
    }
}

void watchDog :: feed_dog(){
    killTimer(dog);
    dog = startTimer(timeout);
}

void watchDog :: dog_barking(){
    this->barking();
}

void watchDog :: timerEvent(QTimerEvent *event){
    if (event->timerId() == dog){
        barking();
    }
}
