#include "postureLogic.h"
#include <cstdio>

int main() {
    // ── Test 1: original accel-only API ──────────────────────────────────────
    printf("=== Test 1: accel-only (original) ===\n");
    AccelData accel = {0.15f, 0.17f, 0.23f};
    printf("Pitch: %f\n", calculatePitch(accel));
    printf("Roll:  %f\n", calculateRoll(accel));

    // ── Test 2: Kalman-filtered API ───────────────────────────────────────────
    printf("\n=== Test 2: Kalman-filtered (accel + gyro) ===\n");

    const float dt = 1.0f / 125.0f;   // matches your 125 Hz sample rate
    AccelData simAccel = {0.05f, 0.0f, 0.999f};  // ~2.9 deg pitch
    GyroData  simGyro  = {0.0f,  1.0f, 0.0f};    // 1 deg/s pitch rate

    KalmanPostureEstimator estimator;
    estimator.init(simAccel);

    for (int i = 0; i < 250; ++i) {   // simulate 2 seconds
        PostureState state = estimator.update(simAccel, simGyro, dt, 0.0f, 0.0f);
        if (i == 0 || i == 124 || i == 249) {
            printf("t=%.2fs  pitch=%.2f  roll=%.2f  posture=%s\n",
                   i * dt, state.pitch, state.roll,
                   state.postureGood ? "GOOD" : "BAD");
        }
    }

    // ── Test 3: threshold boundaries ─────────────────────────────────────────
    printf("\n=== Test 3: isAngleGood ===\n");
    printf("9.9  deg -> %s (expect GOOD)\n", isAngleGood( 9.9f, 0.0f) ? "GOOD" : "BAD");
    printf("10.1 deg -> %s (expect BAD)\n",  isAngleGood(10.1f, 0.0f) ? "GOOD" : "BAD");

    return 0;
}
