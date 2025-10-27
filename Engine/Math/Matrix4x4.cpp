#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "SmoothieMath.h"
#include <iostream>
#include <cmath>
using namespace SmoothieMath;

Matrix4x4::Matrix4x4( const Vector4& row0, const Vector4& row1, const Vector4& row2, const Vector4& row3)
{
	column0.x = row0.x;
	column1.x = row0.y;
	column2.x = row0.z;
	column3.x = row0.w;

	column0.y = row1.x;
	column1.y = row1.y;
	column2.y = row1.z;
	column3.y = row1.w;

	column0.z = row2.x;
	column1.z = row2.y;
	column2.z = row2.z;
	column3.z = row2.w;

	column0.w = row3.x;
	column1.w = row3.y;
	column2.w = row3.z;
	column3.w = row3.w;
}

SmoothieMath::Matrix4x4::Matrix4x4(const Matrix3x3& m)
{
	column0.x = m.column0.x;
	column1.x = m.column1.x;
	column2.x = m.column2.x;
	column3.x = 0;

	column0.y = m.column0.y;
	column1.y = m.column1.y;
	column2.y = m.column2.y;
	column3.y = 0;

	column0.z = m.column0.z;
	column1.z = m.column1.z;
	column2.z = m.column2.z;
	column3.z = 0;

	column0.w = 0;
	column1.w = 0;
	column2.w = 0;
	column3.w = 1.0f;
}

float* Matrix4x4::dataPointer()
{
	return &column0.x;
}

SmoothieMath::Matrix3x3 SmoothieMath::Matrix4x4::normalMatrix() const
{

	/*const float determinant =
		+ row0.x * ((row1.x * row2.y) - (matrix[2][1] * matrix[1][2]))
		- matrix[1][0] * ((matrix[0][1] * matrix[2][2]) - (matrix[2][1] * matrix[0][2]))
		+ matrix[2][0] * ((matrix[0][1] * matrix[1][2]) - (matrix[1][1] * matrix[0][2]));
	if (determinant == 0.0f) return Matrix3x3();
	
	const Vector3 row0 =
	{
		+((matrix[1][1] * matrix[2][2]) - (matrix[2][1] * matrix[1][2])) / determinant,
		-((matrix[0][1] * matrix[2][2]) - (matrix[2][1] * matrix[0][2])) / determinant,
		+((matrix[0][1] * matrix[1][2]) - (matrix[1][1] * matrix[0][2])) / determinant
	};

	const Vector3 row1 =
	{
		-((matrix[1][0] * matrix[2][2]) - (matrix[2][0] * matrix[1][2])) / determinant,
		+((matrix[0][0] * matrix[2][2]) - (matrix[2][0] * matrix[0][2])) / determinant,
		-((matrix[0][0] * matrix[1][2]) - (matrix[1][0] * matrix[0][2])) / determinant
	};

	const Vector3 row2 =
	{
		+((matrix[1][0] * matrix[2][1]) - (matrix[2][0] * matrix[1][1])) / determinant,
		-((matrix[0][0] * matrix[2][1]) - (matrix[2][0] * matrix[0][1])) / determinant,
		+((matrix[0][0] * matrix[1][1]) - (matrix[1][0] * matrix[0][1])) / determinant
	};

	return Matrix3x3(row0, row1, row2);*/
	return Matrix3x3();
}

Matrix3x3 SmoothieMath::Matrix4x4::getRotationMatrix() const
{

	const float s_x = std::sqrt(column0.x * column0.x + column1.x * column1.x + column2.x * column2.x);
	const float s_y = std::sqrt(column0.y * column0.y + column1.y * column1.y + column2.y * column2.y);
	const float s_z = std::sqrt(column0.z * column0.z + column1.z * column1.z + column2.z * column2.z);

	Matrix3x3 m = Matrix3x3();
	m.column0.x = column0.x / s_x;
	m.column0.y = column0.y / s_y;
	m.column0.z = column0.z / s_z;

	m.column1.x = column2.x / s_x;
	m.column1.y = column2.y / s_y;
	m.column1.z = column2.z / s_z;

	m.column2.x = -column1.x / s_x;
	m.column2.y = -column1.y / s_y;
	m.column2.z = -column1.z / s_z;

	return m;
}

//SmoothieMath::Vector3 SmoothieMath::Matrix4x4::getScaleComponent() const
//{
//	const float s_x = std::sqrt(row0.x * row0.x + row0.y * row0.y + row0.z * row0.z);
//	const float s_y = std::sqrt(row1.x * row1.x + row1.y * row1.y + row1.z * row1.z);
//	const float s_z = std::sqrt(row2.x * row2.x + row2.y * row2.y + row2.z * row2.z);
//
//	return SmoothieMath::Vector3(s_x, s_y, s_z);
//}

//SmoothieMath::Vector3 SmoothieMath::Matrix4x4::getEulerAnglesComponent() const
//{
//
//	const float s_x = std::sqrt(row0.x * row0.x + row0.y * row0.y + row0.z * row0.z);
//	const float s_y = std::sqrt(row1.x * row1.x + row1.y * row1.y + row1.z * row1.z);
//	const float s_z = std::sqrt(row2.x * row2.x + row2.y * row2.y + row2.z * row2.z);
//
//	const float r = sqrtf(1.0f + (row0.x / s_x) - (row1.y / s_y) + (row2.y / s_z));
//	
//	const float q_w = r / 2.0f;
//	const float q_y = ((matrix[2][0] / s_z) - (matrix[0][1] / s_x)) / (4.0f * q_w);
//
//	const float q_x = ((matrix[1][0] / s_y) - (matrix[0][2] / s_x)) / (4.0f * q_y);
//	const float q_z = ((matrix[1][1] / s_y) - (matrix[2][2] / s_x)) / (4.0f * q_y);
//
//	const float x = acosf(q_x / sqrtf(1 - q_w * q_w)) * 180.0f / pi;
//	const float y = acosf(q_y / sqrtf(1 - q_w * q_w)) * 180.0f / pi;
//	const float z = acosf(q_z / sqrtf(1 - q_w * q_w)) * 180.0f / pi;
//	
//	return { x, y, z };
//
//	if (std::abs(matrix[0][1] / s_x) != 1.0f)
//	{ 
//		const float Ry = asinf(-matrix[0][1] / s_x);
//		const float Rx = asinf( matrix[1][1] / (s_y * cosf(Ry)));
//		const float Rz = asinf(-matrix[0][2] / (s_x * cosf(Ry)));
//		
//		return 
//		{ 
//			Rx * 180.0f / pi,
//			Ry * 180.0f / pi, 
//			Rz * 180.0f / pi 
//		};
//	}
//	else
//	{
//		std::cout << "Aaaa" << std::endl;
//	}
//	return { 0, 0, 0 };
//	/*if (std::abs(matrix[0][1]) == -1.0f)
//	{
//		const float roll = 0.0f;
//		const float pitch = 90.0f;
//		const float yaw = atan2(matrix[1][0], matrix[2][0]) * 180.0f/pi;
//		return { yaw, pitch, roll };
//	}
//
//	if (std::abs(matrix[0][1]) == 1.0f)
//	{
//		const float roll = 0.0f;
//		const float pitch = -90.0f;
//		const float yaw = atan2(-matrix[1][0], -matrix[2][0]) * 180.0f / pi;
//		return { yaw, pitch, roll };
//	}
//	*/
//}

//SmoothieMath::Vector3 SmoothieMath::Matrix4x4::getPositionComponent() const
//{
//	return SmoothieMath::Vector3(matrix[3][0], -matrix[3][2], matrix[3][1]);
//}

void SmoothieMath::Matrix4x4::transformMatrix(
	const SmoothieMath::Vector3& T, 
	const SmoothieMath::Vector3& R,
	const SmoothieMath::Vector3& S)
{
	const float Rx = R.x * pi / 180.0f;
	const float Ry = R.y * pi / 180.0f;
	const float Rz = R.z * pi / 180.0f;

	const Vector4 row0 = Vector4
	(	
		S.x *  cos(Ry) * cos(Rz),
		S.y * (sin(Rx) * sin(Ry) * cos(Rz) - cos(Rx) * sin(Rz)),
		S.z * (cos(Rx) * sin(Ry) * cos(Rz) + sin(Rx) * sin(Rz)),
		T.x
	);
	
	const Vector4 row1 = Vector4
	(
	   -S.x * sin(Ry),
		S.y * sin(Rx) * cos(Ry),
		S.z * cos(Rx) * cos(Ry),
		T.z
	);

	const Vector4 row2 = Vector4
	(
		-S.x *  cos(Ry) * sin(Rz),
		-S.y * (sin(Rx) * sin(Ry) * sin(Rz) + cos(Rx) * cos(Rz)),
		-S.z * (cos(Rx) * sin(Ry) * sin(Rz) - sin(Rx) * cos(Rz)),
		-T.y
	);

	const Vector4 row3 = { 0.0f, 0.0f, 0.0f, 1.0f };

	column0.x = row0.x;
	column1.x = row0.y;
	column2.x = row0.z;
	column3.x = row0.w;

	column0.y = row1.x;
	column1.y = row1.y;
	column2.y = row1.z;
	column3.y = row1.w;

	column0.z = row2.x;
	column1.z = row2.y;
	column2.z = row2.z;
	column3.z = row2.w;

	column0.w = row3.x;
	column1.w = row3.y;
	column2.w = row3.z;
	column3.w = row3.w;

}

void SmoothieMath::Matrix4x4::transformMatrix(const SmoothieMath::Vector3& position, const SmoothieMath::Matrix3x3& rotationMatrix, const SmoothieMath::Vector3& scale)
{

}

//Matrix4x4 SmoothieMath::Matrix4x4::operator*(float scalar)
//{
//	matrix[0][0] *= scalar, matrix[0][1] *= scalar, matrix[0][2] *= scalar, matrix[0][3] *= scalar,
//	matrix[1][0] *= scalar, matrix[1][1] *= scalar, matrix[1][2] *= scalar, matrix[1][3] *= scalar,
//	matrix[2][0] *= scalar, matrix[2][1] *= scalar, matrix[2][2] *= scalar, matrix[2][3] *= scalar,
//	matrix[3][0] *= scalar, matrix[3][1] *= scalar, matrix[3][2] *= scalar, matrix[3][3] *= scalar;
//	return matrix;
//}

void Matrix4x4::scaleMatrix(float x, float y, float z)
{
	column0.x = x;
	column1.y = y;
	column2.z = z;
}

void Matrix4x4::translateMatrix( float x,  float y, float z)
{
	column3.x = x;
	column3.y = y;
	column3.z = z;
}

void Matrix4x4::orthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	float r_l = right - left;
	if (r_l == 0) return;
	
	float t_b = top - bottom;
	if (t_b == 0) return;

	float f_n = zFar - zNear;
	if (f_n == 0) return;

	column0.x = 2 / r_l;
	column1.y = 2 / t_b;
	column2.z = -2 / f_n;

	column3.x = -(right + left) / r_l;
	column3.y = -(top + bottom) / t_b;
	column3.z = -(zFar + zNear) / f_n;

}

void Matrix4x4::perspectiveProjection( float fovy, float aspec, float zNear, float zFar)
{	
	const float toRadians = fovy*pi/180;
	const float f = 1/std::tan(toRadians/2);
	column0.x = f / aspec;
	column1.y = f;

	const float value = zNear - zFar;
	column2.z = (zFar + zNear) / value;
	
	column3.z = (2 * zFar * zNear) / value;
	column2.w = -1;
	column3.w = 0;
}

//void Matrix4x4::xAxisRotationDegrees( float degrees)
//{
//	float value = degrees * pi / 180;
//	matrix[1][1] = std::cos(value); matrix[2][1] = -std::sin(value);
//	matrix[1][2] = std::sin(value); matrix[2][2] =  std::cos(value);
//}
//
//void Matrix4x4::xAxisRotationRadians( float radians)
//{
//	matrix[1][1] = std::cos(radians); matrix[2][1] = -std::sin(radians);
//	matrix[1][2] = std::sin(radians); matrix[2][2] =  std::cos(radians);
//}
//
//void Matrix4x4::yAxisRotationDegrees( float degrees)
//{
//	float value = degrees * pi / 180;
//	matrix[0][0] = std::cos(value); matrix[0][2] = std::sin(value);
//	matrix[2][0] = -std::sin(value); matrix[2][2] = std::cos(value);
//}
//
//void Matrix4x4::yAxisRotationRadians( float radians)
//{
//	matrix[0][0] = std::cos(radians); matrix[0][2] = std::sin(radians);
//	matrix[2][0] = -std::sin(radians); matrix[2][2] = std::cos(radians);
//}
//
//void Matrix4x4::zAxisRotationDegrees( float degrees)
//{
//	float value = degrees * pi / 180;
//	matrix[0][0] = std::cos(value); matrix[0][1] = -std::sin(value);
//	matrix[1][0] = std::sin(value); matrix[1][1] = std::cos(value);
//}
//
//void Matrix4x4::zAxisRotationRadians( float radians)
//{
//	matrix[0][0] = std::cos(radians); matrix[0][1] = -std::sin(radians);
//	matrix[1][0] = std::sin(radians); matrix[1][1] = std::cos(radians);
//}

void Matrix4x4::lookAtMatrix(const Vector3& eye, const Vector3& center, const Vector3& up)
{
	Vector3 f = Vector3(center - eye);
	f.normalizeVector();
	Vector3 s = cross(f, up);
	s.normalizeVector();
	Vector3 u = cross( s, f);

	column0.x = s.x;
	column1.x = s.y;
	column2.x = s.z;

	column0.y = u.x;
	column1.y = u.y;
	column2.y = u.z;

	column0.z = -f.x;
	column1.z = -f.y;
	column2.z = -f.z;

	column3.x = -dot(s, eye);
	column3.y = -dot(u, eye);
	column3.z = dot(f, eye);

}

void SmoothieMath::transpose(Matrix4x4& matrix)
{
	const Vector4 column0 = matrix.column0;
	const Vector4 column1 = matrix.column1;
	const Vector4 column2 = matrix.column2;
	const Vector4 column3 = matrix.column3;

	matrix = Matrix4x4(
		{ column0.x, column1.x, column2.x, column3.x },
		{ column0.y, column1.y, column2.y, column3.y },
		{ column0.z, column1.z, column2.z, column3.z },
		{ column0.w, column1.w, column2.w, column3.w }
	);

}

Vector4 SmoothieMath::operator*(const Matrix4x4& matrix, const Vector4& v)
{
	const float m0 =
		(matrix.column0.x * v.x) +
		(matrix.column1.x * v.y) +
		(matrix.column2.x * v.z) +
		(matrix.column3.x * v.w);
	
	const float m1 =
		(matrix.column0.y * v.x) +
		(matrix.column1.y * v.y) +
		(matrix.column2.y * v.z) +
		(matrix.column3.y * v.w);

	const float m2 =
		(matrix.column0.z * v.x) +
		(matrix.column1.z * v.y) +
		(matrix.column2.z * v.z) +
		(matrix.column3.z * v.w);

	const float m3 =
		(matrix.column0.w * v.x) +
		(matrix.column1.w * v.y) +
		(matrix.column2.w * v.z) +
		(matrix.column3.w * v.w);

	
	return {m0, m1, m2, m3};
}

Matrix4x4 SmoothieMath::operator*(const Matrix4x4& A, const Matrix4x4& B)
{

	const Vector4 row0 = { A.column0.x,  A.column1.x, A.column2.x, A.column3.x };
	const Vector4 row1 = { A.column0.y,  A.column1.y, A.column2.y, A.column3.y };
	const Vector4 row2 = { A.column0.z,  A.column1.z, A.column2.z, A.column3.z };
	const Vector4 row3 = { A.column0.w,  A.column1.w, A.column2.w, A.column3.w };

	const float m00 = dot(B.column0, row0);
	const float m01 = dot(B.column0, row1);
	const float m02 = dot(B.column0, row2);
	const float m03 = dot(B.column0, row3);

	const float m10 = dot(B.column1, row0);
	const float m11 = dot(B.column1, row1);
	const float m12 = dot(B.column1, row2);
	const float m13 = dot(B.column1, row3);

	const float m20 = dot(B.column2, row0);
	const float m21 = dot(B.column2, row1);
	const float m22 = dot(B.column2, row2);
	const float m23 = dot(B.column2, row3);

	const float m30 = dot(B.column3, row0);
	const float m31 = dot(B.column3, row1);
	const float m32 = dot(B.column3, row2);
	const float m33 = dot(B.column3, row3);

	return Matrix4x4(
		{ m00, m10, m20, m30 },
		{ m01, m11, m21, m31 },
		{ m02, m12, m22, m32 },
		{ m03, m13, m23, m33 });
}

Matrix4x4 SmoothieMath::operator * (float scalar, const Matrix4x4& matrix)
{
	return Matrix4x4();
}

Matrix4x4 SmoothieMath::inverse(const Matrix4x4& matrix)
{
	const auto& c0 = matrix.column0;
	const auto& c1 = matrix.column1;
	const auto& c2 = matrix.column2;
	const auto& c3 = matrix.column3;
	
	const float f1  = c2.z * c3.w - c2.w * c3.z;
	const float f2  = c2.y * c3.w - c2.w * c3.y;
	const float f3  = c2.y * c3.z - c2.z * c3.y;
	const float f4  = c2.x * c3.w - c2.w * c3.x;
	const float f5  = c2.x * c3.z - c2.z * c3.x;
	const float f6  = c2.x * c3.y - c2.y * c3.x;
	const float f7  = c1.z * c3.w - c1.w * c3.z;
	const float f8  = c1.y * c3.w - c1.w * c3.y;
	const float f9  = c1.y * c3.z - c1.z * c3.y;
	const float f10 = c1.x * c3.w - c1.w * c3.x;
	const float f11 = c1.x * c3.z - c1.z * c3.x;
	const float f13 = c1.x * c3.y - c1.y * c3.x;
	const float f14 = c1.z * c2.w - c1.w * c2.z;
	const float f15 = c1.y * c2.w - c1.w * c2.y;
	const float f16 = c1.y * c2.z - c1.z * c2.y;
	const float f17 = c1.x * c2.w - c1.w * c2.x;
	const float f18 = c1.x * c2.z - c1.z * c2.x;
	const float f19 = c1.x * c2.y - c1.y * c2.x;
	
	const float K0x = +(c1.y * f1  - c1.z * f2  + c1.w * f3);
	const float K0y = -(c1.x * f1  - c1.z * f4  + c1.w * f5);
	const float K0z = +(c1.x * f2  - c1.y * f4  + c1.w * f6);
	const float K0w = -(c1.x * f3  - c1.y * f5  + c1.z * f6);

	const float K1x = -(c0.y * f1  - c0.z * f2  + c0.w * f3);
	const float K1y = +(c0.x * f1  - c0.z * f4  + c0.w * f5);
	const float K1z = -(c0.x * f2  - c0.y * f4  + c0.w * f6);
	const float K1w = +(c0.x * f3  - c0.y * f5  + c0.z * f6);

	const float K2x = +(c0.y * f7  - c0.z * f8  + c0.w * f9);
	const float K2y = -(c0.x * f7  - c0.z * f10 + c0.w * f11);
	const float K2z = +(c0.x * f8  - c0.y * f10 + c0.w * f13);
	const float K2w = -(c0.x * f9  - c0.y * f11 + c0.z * f13);

	const float K3x = -(c0.y * f14 - c0.z * f15 + c0.w * f16);
	const float K3y = +(c0.x * f14 - c0.z * f17 + c0.w * f18);
	const float K3z = -(c0.x * f15 - c0.y * f17 + c0.w * f19);
	const float K3w = +(c0.x * f16 - c0.y * f18 + c0.z * f19);

	//one over determinant
	const float ood = 1.0f / dot({K0x, K0y, K0z, K0w}, { c0.x, c0.y, c0.z, c0.w });

	const Vector4 K0 = { K0x * ood, K0y * ood, K0z * ood, K0w * ood };
	const Vector4 K1 = { K1x * ood, K1y * ood, K1z * ood, K1w * ood };
	const Vector4 K2 = { K2x * ood, K2y * ood, K2z * ood, K2w * ood };
	const Vector4 K3 = { K3x * ood, K3y * ood, K3z * ood, K3w * ood };

	return Matrix4x4(K0, K1, K2, K3);
}
