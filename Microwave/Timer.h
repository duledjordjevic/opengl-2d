#ifndef TIMER_H
#define TIMER_H

#include <iostream>

struct Timer {
    int time[4];
    int counter = 0;
    bool running = false;
    bool paused = false;

    void setNextValue(int value);
};

#endif 
