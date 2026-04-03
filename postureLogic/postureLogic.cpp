#include "postureLogic.h"
#include <cmath>

static const float THRESHOLD = 10.0f;
static const float PI = 3.14159265f;

float calculatePitch(const AccelData& accel) {
    return (atan2(accel.x, sqrt(accel.y * accel.y + accel.z * accel.z)) * 180.0f) / PI;
}

float calculateRoll(const AccelData& accel) {
    return (atan2(accel.y, accel.z) * 180.0f) / PI;
}

bool isAngleGood(float angle, float defaultAngle) {
    const float lowerThreshold = defaultAngle - THRESHOLD;
    const float upperThreshold = defaultAngle + THRESHOLD;

    return (angle >= lowerThreshold && angle <= upperThreshold);
}

PostureState evaluatePosture(const AccelData& accel, float defaultPitch, float defaultRoll) {
    PostureState state;

    state.accel = accel;
    state.pitch = calculatePitch(accel);
    state.roll = calculateRoll(accel);

    state.pitchGood = isAngleGood(state.pitch, defaultPitch);
    state.rollGood = isAngleGood(state.roll, defaultRoll);

    state.postureGood = state.pitchGood && state.rollGood;

    return state;
}