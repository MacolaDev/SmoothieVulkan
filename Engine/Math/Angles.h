#pragma once

namespace SmoothieMath 
{
	struct EulerAngles
	{
		float pitch = 0.0f, yaw = 0.0f, roll = 0.0f;
	};

	struct Quaternion
	{
		float w = 0.0f, x = 0.0f, y = 0.0f, z = 0.0f;
	};
	
	EulerAngles ToEulerAngles(Quaternion quaternion);

	Quaternion ToQuaternion(EulerAngles angle);
}