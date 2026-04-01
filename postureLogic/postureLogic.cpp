#include "postureLogic.h"
#include <cmath>

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
    const float pitchLowerThreshold = baselinePitch - 10.0f;
    const float pitchUpperThreshold = baselinePitch + 10.0f;
    const float rollLowerThreshold = baselineRoll - 10.0f;
    const float rollUpperThreshold = baselineRoll + 10.0f;

    bool pitchGood = (currentPitch >= pitchLowerThreshold) && (currentPitch <= pitchUpperThreshold);
    bool rollGood = (currentRoll >= rollLowerThreshold) && (currentRoll <= rollUpperThreshold);
    return pitchGood && rollGood;
}






