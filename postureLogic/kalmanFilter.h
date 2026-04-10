class KalmanFilter {
public:
    KalmanFilter();

    float update(float accel_angle, float gyro_rate, float dt);

    void setAngle(float angle);

    float getAngle() const { return angle; }
    float getBias()  const { return bias;  }

    float Q_angle;
    float Q_bias;
    float R_measure;

private:
    float angle;
    float bias;
    float P[2][2];
};