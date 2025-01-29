#include "Matrix3x3.h"

SmoothieMath::Matrix3x3::Matrix3x3()
{
	matrix3[0][0] = 1.0f, matrix3[0][1] = 0.0f, matrix3[0][2] = 0.0f;
	matrix3[1][0] = 0.0f, matrix3[1][1] = 1.0f, matrix3[1][2] = 0.0f;
	matrix3[2][0] = 0.0f, matrix3[2][1] = 0.0f, matrix3[2][2] = 1.0f;
}

SmoothieMath::Matrix3x3::Matrix3x3(const Vector3& row0, const Vector3& row1, const Vector3& row2)
{
	matrix3[0][0] = row0.x, matrix3[1][0] = row0.y, matrix3[2][0] = row0.z;
	matrix3[0][1] = row1.x, matrix3[1][1] = row1.y, matrix3[2][1] = row1.z;
	matrix3[0][2] = row2.x, matrix3[1][2] = row2.y, matrix3[2][2] = row2.z;
}

SmoothieMath::Matrix3x3::Matrix3x3(const Matrix4x4& m)
{
	matrix3[0][0] = m.matrix[0][0], matrix3[0][1] = m.matrix[0][1], matrix3[0][2] = m.matrix[0][2];
	matrix3[1][0] = m.matrix[1][0], matrix3[1][1] = m.matrix[1][1], matrix3[1][2] = m.matrix[1][2];
	matrix3[2][0] = m.matrix[2][0], matrix3[2][1] = m.matrix[2][1], matrix3[2][2] = m.matrix[2][2];
}

float* SmoothieMath::Matrix3x3::dataPointer()
{
	return &matrix3[0][0];
}
