#pragma once
#include "Vector4.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Angles.h"
#include "Vector2.h"

//Math used in Smoothie render engine.
namespace SmoothieMath {
	const float pi = 3.141592653589f;
	
	inline float toRadians(float degrees) 
	{
		return (pi * degrees) / 180.0f;
	}
}
