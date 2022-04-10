#ifdef I2C_GAZE_CONTROL_H
#define I2C_GAZE_CONTROL_H

extern void UpdateEyePosition(float eyeX, float eyeY);
extern float GetScaledEyeTargetX();
extern float GetScaledEyeTargetY();
#endif