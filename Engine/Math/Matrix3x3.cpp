#include "Matrix3x3.h"
#include "Matrix4x4.h"

SmoothieMath::Matrix3x3::Matrix3x3(const Vector3& row0, const Vector3& row1, const Vector3& row2)
{
	column0.x = row0.x;
	column1.x = row0.y;
	column2.x = row0.z;

	column0.y = row1.x;
	column1.y = row1.y;
	column2.y = row1.z;

	column0.z = row2.x;
	column1.z = row2.y;
	column2.z = row2.z;
}

SmoothieMath::Matrix3x3::Matrix3x3(const Matrix4x4& m)
{
	column0 = { m.column0.x, m.column0.y, m.column0.z };
	column1 = { m.column1.x, m.column1.y, m.column1.z };
	column2 = { m.column2.x, m.column2.y, m.column1.z };
}

float* SmoothieMath::Matrix3x3::dataPointer()
{
	return &column0.x;
}
