#ifndef POSTURE_LOGIC_H
#define POSTURE_LOGIC_H

#include "mpu_6050.h"

constexpr float kPostureThresholdDegrees = 10.0f;

float calculatePitch(const AccelData& accel);
float calculateRoll(const AccelData& accel);
bool isPostureGood(float currentPitch, float baselinePitch, float currentRoll, float baselineRoll);

#endif
