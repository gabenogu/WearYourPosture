#ifndef POSTURE_LOGIC_H
#define POSTURE_LOGIC_H

struct AccelData {
    float x;
    float y;
    float z;
};

float calculatePitch(const AccelData& accel);
float calculateRoll(const AccelData& accel);
bool isPostureGood(float tilt, float defaultDeg);

#endif