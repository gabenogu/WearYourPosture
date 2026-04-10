#ifndef POSTURE_LOGIC_H
#define POSTURE_LOGIC_H

#include "kalmanFilter.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ── Raw sensor data structs ───────────────────────────────────────────────────

struct AccelData {
    float x;
    float y;
    float z;
};

struct GyroData {
    float x;  // deg/s  (roll  rate)
    float y;  // deg/s  (pitch rate)
    float z;  // deg/s  (yaw   rate, unused for posture)
};

// ── Result struct ─────────────────────────────────────────────────────────────

struct PostureState {
    AccelData accel;
    float pitch;
    float roll;
    bool pitchGood;
    bool rollGood;
    bool postureGood;
};

// ── Stateless helpers (accel-only, kept for compatibility) ────────────────────

float calculatePitch(const AccelData& accel);
float calculateRoll (const AccelData& accel);
bool  isAngleGood   (float angle, float defaultAngle);

// Original accel-only evaluation — unchanged API
PostureState evaluatePosture(const AccelData& accel,
                             float defaultPitch,
                             float defaultRoll);

// ── KalmanPostureEstimator — stateful, thread-safe, fuses accel + gyro ────────
/**
 * Thread-safe Kalman-filtered posture estimator.
 *
 * The internal state (pitch, roll, filter covariance) is protected by a
 * FreeRTOS mutex so the sensor task and Bluetooth task can't corrupt each
 * other's reads/writes.
 *
 * Sensor task calls:  update()       — writes new filtered angles
 * Bluetooth task calls: getState()   — reads the latest PostureState safely
 */
class KalmanPostureEstimator {
public:
    KalmanPostureEstimator();
    ~KalmanPostureEstimator();

    /** Seed both filters from an initial accelerometer reading. */
    void init(const AccelData& accel);

    /**
     * Fuse one sensor sample and update internal state.
     * Called by the sensor/multiplexer task.
     *
     * @param accel        Raw accelerometer reading (g)
     * @param gyro         Raw gyroscope reading (deg/s)
     * @param dt           Time since last call (seconds)
     * @param defaultPitch Calibrated "good" pitch (degrees)
     * @param defaultRoll  Calibrated "good" roll  (degrees)
     */
    void update(const AccelData& accel,
                const GyroData&  gyro,
                float            dt,
                float            defaultPitch,
                float            defaultRoll);

    /**
     * Safely read the latest posture state.
     * Called by the Bluetooth task — will never race with update().
     */
    PostureState getState();

    // Expose filters so caller can tune Q/R values if desired
    KalmanFilter pitchFilter;
    KalmanFilter rollFilter;

private:
    PostureState  latestState;   // protected by mutex
    SemaphoreHandle_t mutex;     // FreeRTOS mutex
};

#endif // POSTURE_LOGIC_H
