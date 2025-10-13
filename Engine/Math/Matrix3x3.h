#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"

namespace SmoothieMath {
	struct Matrix4x4;
	struct Matrix3x3
	{
		Vector3 column0 = { 1.0f, 0.0f, 0.0f };
		Vector3 column1 = { 0.0f, 1.0f, 0.0f };
		Vector3 column2 = { 0.0f, 0.0f, 1.0f };

		Matrix3x3() = default;
		Matrix3x3(const Vector3& column0, const Vector3& row1, const Vector3& row2);
		Matrix3x3(const Matrix4x4& matrix);

		float* dataPointer();
		
	};

}