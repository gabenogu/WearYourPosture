#ifndef POSTURE_LOGIC_H
#define POSTURE_LOGIC_H

#include "mpu_6050.h"

float calculatePitch(const AccelData& accel);
float calculateRoll(const AccelData& accel);
bool isPostureGood(float currentPitch, float baselinePitch, float currentRoll, float baselineRoll);

#endif
