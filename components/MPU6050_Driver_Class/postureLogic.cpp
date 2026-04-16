#include "include/postureLogic.h"
#include <cmath>

constexpr float ACCEL_X_UPRIGHT = -0.70f;
constexpr float ACCEL_Y_UPRIGHT = 0.15f;
constexpr float ACCEL_Z_UPRIGHT = 0.65;
// How to do tests with ESP-IDF on VSCode
// lets you compare values at compile time and test if 
// system / functions are working as intended
float calculatePitch(const AccelData& accel) {
    return (atan2(accel.x, sqrt(accel.y * accel.y  + accel.z * accel.z)) * 180) / M_PI;
    
}



float calculateRoll(const AccelData& accel){
    return (atan2(accel.y, accel.z) * 180) / M_PI;
}


//with calibration tilt in mind, take current tilt deg, set both to abs
//check if current tilt is within acceptable threshold
bool isPostureGood(float currentPitch, float baselinePitch, float currentRoll, float baselineRoll){
    const float pitchLowerThreshold = baselinePitch - kPostureThresholdDegrees;
    const float pitchUpperThreshold = baselinePitch + kPostureThresholdDegrees;
    const float rollLowerThreshold = baselineRoll - kPostureThresholdDegrees;
    const float rollUpperThreshold = baselineRoll + kPostureThresholdDegrees;

    bool pitchGood = (currentPitch >= pitchLowerThreshold) && (currentPitch <= pitchUpperThreshold);
    bool rollGood = (currentRoll >= rollLowerThreshold) && (currentRoll <= rollUpperThreshold);
    return pitchGood && rollGood;
}





