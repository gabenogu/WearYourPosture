#ifndef POSTURE_LOGIC_H
#define POSTURE_LOGIC_H

struct AccelData {
    float x;
    float y;
    float z;
};

struct PostureState {
    AccelData accel;
    float pitch;
    float roll;
    bool pitchGood;
    bool rollGood;
    bool postureGood;
};

float calculatePitch(const AccelData& accel);
float calculateRoll(const AccelData& accel);
bool isAngleGood(float angle, float defaultAngle);
PostureState evaluatePosture(const AccelData& accel, float defaultPitch, float defaultRoll);

#endif