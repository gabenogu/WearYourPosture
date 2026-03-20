#include "postureLogic.h"
#include <iostream>
#include <cassert>
//espidf assert 
int main() {
    AccelData Accel = {0.15, 0.17, 0.23};

    float pitch = calculatePitch(Accel);
    float roll = calculateRoll(Accel);
    printf("%f\n, %f\n", pitch, roll);
    return 0;
}