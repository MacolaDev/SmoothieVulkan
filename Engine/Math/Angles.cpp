#include "Angles.h"
#include <math.h>
#include "SmoothieMath.h"
using namespace SmoothieMath;
EulerAngles SmoothieMath::ToEulerAngles(Quaternion q)
{
    EulerAngles angles;

    const float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    const float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    angles.roll = atan2f(sinr_cosp, cosr_cosp);
    angles.roll *= 180.0f / pi;

    const float sinp = sqrtf(1 + 2 * (q.w * q.y - q.x * q.z));
    const float cosp = sqrtf(1 - 2 * (q.w * q.y - q.x * q.z));
    angles.pitch = 2 * atan2f(sinp, cosp) - pi/ 2;
    angles.pitch *= 180.0f / pi;

    const float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    const float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    angles.yaw = atan2f(siny_cosp, cosy_cosp);
    angles.yaw *= 180.0f / pi;

    return angles;
}

Quaternion SmoothieMath::ToQuaternion(EulerAngles angle)
{

    const float cr = cosf(angle.pitch * 0.5f * pi/180.f);
    const float sr = sinf(angle.pitch * 0.5f * pi / 180.f);
    const float cp = cosf(angle.yaw * 0.5f * pi / 180.f);
    const float sp = sinf(angle.yaw * 0.5f * pi / 180.f);
    const float cy = cosf(angle.roll * 0.5f * pi / 180.f);
    const float sy = sinf(angle.roll * 0.5f * pi / 180.f);

    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
	return q;
}
