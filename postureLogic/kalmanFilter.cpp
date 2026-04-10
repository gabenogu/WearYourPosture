#include "KalmanFilter.h"

KalmanFilter::KalmanFilter()
    : Q_angle(0.001f),
      Q_bias(0.003f),
      R_measure(0.03f),
      angle(0.0f),
      bias(0.0f)
{
    // Initialise error covariance matrix to identity
    P[0][0] = 1.0f; P[0][1] = 0.0f;
    P[1][0] = 0.0f; P[1][1] = 1.0f;
}

void KalmanFilter::setAngle(float startAngle) {
    angle = startAngle;
}

float KalmanFilter::update(float accel_angle, float gyro_rate, float dt) {

    // ── PREDICT ──────────────────────────────────────────────────────────────
    // 1. Integrate gyro rate to get predicted angle, correcting for bias
    float rate = gyro_rate - bias;
    angle += dt * rate;

    // 2. Update error covariance matrix
    P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
    P[0][1] -= dt * P[1][1];
    P[1][0] -= dt * P[1][1];
    P[1][1] += Q_bias * dt;

    // ── UPDATE ───────────────────────────────────────────────────────────────
    // 3. Compute Kalman gain
    float S = P[0][0] + R_measure;          // Innovation covariance
    float K[2];
    K[0] = P[0][0] / S;                     // Gain for angle
    K[1] = P[1][0] / S;                     // Gain for bias

    // 4. Correct the estimate with the accelerometer measurement
    float innovation = accel_angle - angle;  // Difference between measured and predicted
    angle += K[0] * innovation;
    bias  += K[1] * innovation;

    // 5. Update error covariance
    float P00_temp = P[0][0];
    float P01_temp = P[0][1];
    P[0][0] -= K[0] * P00_temp;
    P[0][1] -= K[0] * P01_temp;
    P[1][0] -= K[1] * P00_temp;
    P[1][1] -= K[1] * P01_temp;

    return angle;
}
