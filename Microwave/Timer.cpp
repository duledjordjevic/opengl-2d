#include "Timer.h"

void Timer::setNextValue(int value) {
    if (value < 0 || value > 9) {
        std::cout << "Invalid digit: " << value << ". Must be between 0 and 9." << std::endl;
        return;
    }

    int prev = time[counter];
    time[counter] = value;

    int hh = time[0] * 10 + time[1];
    int ss = time[2] * 10 + time[3];
    if ((counter < 2 && hh > 59) || (counter >= 2 && ss > 59)) {
        std::cout << "Invalid combination: ";
        if (counter < 2) std::cout << "hh = " << hh << std::endl;
        else std::cout << "ss = " << ss << std::endl;

        time[counter] = prev;
        return;
    }

    counter = (counter + 1) % 4;
}
