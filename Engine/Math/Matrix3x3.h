#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"

namespace SmoothieMath {
	class Matrix4x4;
	class Matrix3x3
	{
		float matrix3[3][3];
	public:
		Matrix3x3();
		Matrix3x3(const Vector3& row0, const Vector3& row1, const Vector3& row2);
		Matrix3x3(const Matrix4x4& matrix);

		float* dataPointer();
		friend class Matrix4x4;
		
	};

}