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
    return (angle >= defaultAngle - THRESHOLD &&
            angle <= defaultAngle + THRESHOLD);
}

PostureState evaluatePosture(const AccelData& accel,
                             float defaultPitch,
                             float defaultRoll) {
    PostureState state;
    state.accel       = accel;
    state.pitch       = calculatePitch(accel);
    state.roll        = calculateRoll(accel);
    state.pitchGood   = isAngleGood(state.pitch, defaultPitch);
    state.rollGood    = isAngleGood(state.roll,  defaultRoll);
    state.postureGood = state.pitchGood && state.rollGood;
    return state;
}

// ── KalmanPostureEstimator ────────────────────────────────────────────────────

KalmanPostureEstimator::KalmanPostureEstimator() {
    // Create the mutex — has to  be done before any task uses this object
    mutex = xSemaphoreCreateMutex();
    latestState = {};
}

KalmanPostureEstimator::~KalmanPostureEstimator() {
    if (mutex != nullptr) {
        vSemaphoreDelete(mutex);
    }
}

void KalmanPostureEstimator::init(const AccelData& accel) {
    pitchFilter.setAngle(calculatePitch(accel));
    rollFilter.setAngle (calculateRoll (accel));
}

void KalmanPostureEstimator::update(const AccelData& accel,
                                    const GyroData&  gyro,
                                    float            dt,
                                    float            defaultPitch,
                                    float            defaultRoll) {
    // Run the filter math OUTSIDE the lock 
    // and doesn't touch shared state
    float accelPitch = calculatePitch(accel);
    float accelRoll  = calculateRoll(accel);

    float filteredPitch = pitchFilter.update(accelPitch, gyro.y, dt);
    float filteredRoll  = rollFilter.update(accelRoll,  gyro.x, dt);

    // Build the result
    PostureState state;
    state.accel       = accel;
    state.pitch       = filteredPitch;
    state.roll        = filteredRoll;
    state.pitchGood   = isAngleGood(state.pitch, defaultPitch);
    state.rollGood    = isAngleGood(state.roll,  defaultRoll);
    state.postureGood = state.pitchGood && state.rollGood;

    // Only lock long enough to swap in the new result
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        latestState = state;
        xSemaphoreGive(mutex);
    }
}

PostureState KalmanPostureEstimator::getState() {
    PostureState snapshot;
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        snapshot = latestState;
        xSemaphoreGive(mutex);
    }
    return snapshot;
}
