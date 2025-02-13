#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "SmoothieMath.h"
#include <iostream>

using namespace SmoothieMath;

Matrix4x4::Matrix4x4()
{
	matrix[0][0] = 1, matrix[0][1] = 0, matrix[0][2] = 0, matrix[0][3] = 0,
	matrix[1][0] = 0, matrix[1][1] = 1, matrix[1][2] = 0, matrix[1][3] = 0,
	matrix[2][0] = 0, matrix[2][1] = 0, matrix[2][2] = 1, matrix[2][3] = 0,
	matrix[3][0] = 0, matrix[3][1] = 0, matrix[3][2] = 0, matrix[3][3] = 1;
}

Matrix4x4::Matrix4x4( const Vector4& row0, const Vector4& row1, const Vector4& row2, const Vector4& row3)
{
	matrix[0][0] = row0.x, matrix[1][0] = row0.y, matrix[2][0] = row0.z, matrix[3][0] = row0.w;
	matrix[0][1] = row1.x, matrix[1][1] = row1.y, matrix[2][1] = row1.z, matrix[3][1] = row1.w;
	matrix[0][2] = row2.x, matrix[1][2] = row2.y, matrix[2][2] = row2.z, matrix[3][2] = row2.w;
	matrix[0][3] = row3.x, matrix[1][3] = row3.y, matrix[2][3] = row3.z, matrix[3][3] = row3.w;
}

SmoothieMath::Matrix4x4::Matrix4x4(float data[4][4])
{
	matrix[0][0] = data[0][0];
	matrix[1][0] = data[1][0];
	matrix[2][0] = data[2][0];
	matrix[3][0] = data[3][0];

	matrix[0][1] = data[0][1];
	matrix[1][1] = data[1][1];
	matrix[2][1] = data[2][1];
	matrix[3][1] = data[3][1];

	matrix[0][2] = data[0][2];
	matrix[1][2] = data[1][2];
	matrix[2][2] = data[2][2];
	matrix[3][2] = data[3][2];

	matrix[0][3] = data[0][3];
	matrix[1][3] = data[1][3];
	matrix[2][3] = data[2][3];
	matrix[3][3] = data[3][3];
}

SmoothieMath::Matrix4x4::Matrix4x4(const Matrix3x3& m)
{
	matrix[0][0] = m.matrix3[0][0];
	matrix[1][0] = m.matrix3[1][0];
	matrix[2][0] = m.matrix3[2][0];
	matrix[3][0] = 0;

	matrix[0][1] = m.matrix3[0][1];
	matrix[1][1] = m.matrix3[1][1];
	matrix[2][1] = m.matrix3[2][1];
	matrix[3][1] = 0;

	matrix[0][2] = m.matrix3[0][2];
	matrix[1][2] = m.matrix3[1][2];
	matrix[2][2] = m.matrix3[2][2];
	matrix[3][2] = 0;

	matrix[0][3] = 0;
	matrix[1][3] = 0;
	matrix[2][3] = 0;
	matrix[3][3] = 1;
}

float* Matrix4x4::dataPointer()
{
	return &matrix[0][0];
}

void SmoothieMath::Matrix4x4::inverse()
{
	auto m = matrix;
	float Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
	float Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
	float Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];
	
	float Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
	float Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
	float Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
	
	float Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
	float Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
	float Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
	
	float Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
	float Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
	float Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];
	
	float Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
	float Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
	float Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];
	
	float Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
	float Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
	float Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

	Vector4 Fac0(Coef00, Coef00, Coef02, Coef03);
	Vector4 Fac1(Coef04, Coef04, Coef06, Coef07);
	Vector4 Fac2(Coef08, Coef08, Coef10, Coef11);
	Vector4 Fac3(Coef12, Coef12, Coef14, Coef15);
	Vector4 Fac4(Coef16, Coef16, Coef18, Coef19);
	Vector4 Fac5(Coef20, Coef20, Coef22, Coef23);

	Vector4 Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
	Vector4 Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
	Vector4 Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
	Vector4 Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

	Vector4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
	Vector4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
	Vector4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
	Vector4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

	Vector4 SignA(+1, -1, +1, -1);
	Vector4 SignB(-1, +1, -1, +1);
	
	Vector4 Row0 = Inv0 * SignA;
	Vector4 Row1 = Inv1 * SignB;
	Vector4 Row2 = Inv2 * SignA;
	Vector4 Row3 = Inv3 * SignB;
	
	
	Vector4 row0(Row0.x, Row1.x, Row2.x, Row3.x);

	Vector4 Dot0(Vector4(m[0]) * row0[0]);
	float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

	//One over determinant
	float ood = 1.0f / (Dot1);
	


	matrix[0][0] = Row0.x * ood, matrix[0][1] = Row0.y * ood, matrix[0][2] = Row0.z * ood, matrix[0][3] = Row0.w * ood;
	matrix[1][0] = Row1.x * ood, matrix[1][1] = Row1.y * ood, matrix[1][2] = Row1.z * ood, matrix[1][3] = Row1.w * ood;
	matrix[2][0] = Row2.x * ood, matrix[2][1] = Row2.y * ood, matrix[2][2] = Row2.z * ood, matrix[2][3] = Row2.w * ood;
	matrix[3][0] = Row3.x * ood, matrix[3][1] = Row3.y * ood, matrix[3][2] = Row3.z * ood, matrix[3][3] = Row3.w * ood;

}

void SmoothieMath::Matrix4x4::transpose()
{	
	float temp[4][4];
	temp[0][0] = matrix[0][0]; 
	temp[0][1] = matrix[1][0]; 
	temp[0][2] = matrix[2][0]; 
	temp[0][3] = matrix[3][0];
							   
	temp[1][0] = matrix[0][1];
	temp[1][1] = matrix[1][1]; 
	temp[1][2] = matrix[2][1]; 
	temp[1][3] = matrix[3][1];
							   
	temp[2][0] = matrix[0][2]; 
	temp[2][1] = matrix[1][2]; 
	temp[2][2] = matrix[2][2]; 
	temp[2][3] = matrix[3][2];
							   
	temp[3][0] = matrix[0][3]; 
	temp[3][1] = matrix[1][3]; 
	temp[3][2] = matrix[2][3]; 
	temp[3][3] = matrix[3][3];

	matrix[0][0] = temp[0][0];
	matrix[0][1] = temp[0][1];
	matrix[0][2] = temp[0][2];
	matrix[0][3] = temp[0][3];

	matrix[1][0] = temp[1][0];
	matrix[1][1] = temp[1][1];
	matrix[1][2] = temp[1][2];
	matrix[1][3] = temp[1][3];

	matrix[2][0] = temp[2][0];
	matrix[2][1] = temp[2][1];
	matrix[2][2] = temp[2][2];
	matrix[2][3] = temp[2][3];

	matrix[3][0] = temp[3][0];
	matrix[3][1] = temp[3][1];
	matrix[3][2] = temp[3][2];
	matrix[3][3] = temp[3][3];
}

SmoothieMath::Matrix3x3 SmoothieMath::Matrix4x4::normalMatrix() const
{

	const float determinant =
		+ matrix[0][0] * ((matrix[1][1] * matrix[2][2]) - (matrix[2][1] * matrix[1][2]))
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

	return Matrix3x3(row0, row1, row2);
}

SmoothieMath::Vector3 SmoothieMath::Matrix4x4::getScaleComponent() const
{
	const float s_x = std::sqrt(matrix[0][0] * matrix[0][0] + matrix[0][1] * matrix[0][1] + matrix[0][2] * matrix[0][2]);
	const float s_y = std::sqrt(matrix[1][0] * matrix[1][0] + matrix[1][1] * matrix[1][1] + matrix[1][2] * matrix[1][2]);
	const float s_z = std::sqrt(matrix[2][0] * matrix[2][0] + matrix[2][1] * matrix[2][1] + matrix[2][2] * matrix[2][2]);

	return SmoothieMath::Vector3(s_x, s_y, s_z);
}

SmoothieMath::Vector3 SmoothieMath::Matrix4x4::getEulerAnglesComponent() const
{

	const float s_x = std::sqrt(matrix[0][0] * matrix[0][0] + matrix[0][1] * matrix[0][1] + matrix[0][2] * matrix[0][2]);
	const float s_y = std::sqrt(matrix[1][0] * matrix[1][0] + matrix[1][1] * matrix[1][1] + matrix[1][2] * matrix[1][2]);
	const float s_z = std::sqrt(matrix[2][0] * matrix[2][0] + matrix[2][1] * matrix[2][1] + matrix[2][2] * matrix[2][2]);

	const float r = sqrtf(1.0f + (matrix[0][0] / s_x) - (matrix[1][2] / s_y) + (matrix[2][1] / s_z));
	
	const float q_w = r / 2.0f;
	const float q_y = ((matrix[2][0] / s_z) - (matrix[0][1] / s_x)) / (4.0f * q_w);

	const float q_x = ((matrix[1][0] / s_y) - (matrix[0][2] / s_x)) / (4.0f * q_y);
	const float q_z = ((matrix[1][1] / s_y) - (matrix[2][2] / s_x)) / (4.0f * q_y);

	const float x = acosf(q_x / sqrtf(1 - q_w * q_w)) * 180.0f / pi;
	const float y = acosf(q_y / sqrtf(1 - q_w * q_w)) * 180.0f / pi;
	const float z = acosf(q_z / sqrtf(1 - q_w * q_w)) * 180.0f / pi;
	
	return { x, y, z };

	if (std::abs(matrix[0][1] / s_x) != 1.0f)
	{ 
		const float Ry = asinf(-matrix[0][1] / s_x);
		const float Rx = asinf( matrix[1][1] / (s_y * cosf(Ry)));
		const float Rz = asinf(-matrix[0][2] / (s_x * cosf(Ry)));
		
		return 
		{ 
			Rx * 180.0f / pi,
			Ry * 180.0f / pi, 
			Rz * 180.0f / pi 
		};
	}
	else
	{
		std::cout << "Aaaa" << std::endl;
	}
	return { 0, 0, 0 };
	/*if (std::abs(matrix[0][1]) == -1.0f)
	{
		const float roll = 0.0f;
		const float pitch = 90.0f;
		const float yaw = atan2(matrix[1][0], matrix[2][0]) * 180.0f/pi;
		return { yaw, pitch, roll };
	}

	if (std::abs(matrix[0][1]) == 1.0f)
	{
		const float roll = 0.0f;
		const float pitch = -90.0f;
		const float yaw = atan2(-matrix[1][0], -matrix[2][0]) * 180.0f / pi;
		return { yaw, pitch, roll };
	}
	*/
}

SmoothieMath::Vector3 SmoothieMath::Matrix4x4::getPositionComponent() const
{
	return SmoothieMath::Vector3(matrix[3][0], -matrix[3][2], matrix[3][1]);
}

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

	matrix[0][0] = row0.x, matrix[1][0] = row0.y, matrix[2][0] = row0.z, matrix[3][0] = row0.w;
	matrix[0][1] = row1.x, matrix[1][1] = row1.y, matrix[2][1] = row1.z, matrix[3][1] = row1.w;
	matrix[0][2] = row2.x, matrix[1][2] = row2.y, matrix[2][2] = row2.z, matrix[3][2] = row2.w;
	matrix[0][3] = row3.x, matrix[1][3] = row3.y, matrix[2][3] = row3.z, matrix[3][3] = row3.w;

}

Matrix4x4 SmoothieMath::Matrix4x4::operator*(const Matrix4x4& m2)
{	

	float result[4][4];
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			result[i][j] = 0.0f;
		}
	}

	for (int i = 0; i < 4; ++i) 
	{
		for (int j = 0; j < 4; ++j) 
		{
			for (int k = 0; k < 4; ++k) 
			{
				result[i][j] += matrix[i][k] * m2[k][j];
			}
		}
	}
	

	return Matrix4x4(result);
}

Vector4 SmoothieMath::Matrix4x4::operator[](int index) const
{	
	return Vector4(matrix[index]);
}

void Matrix4x4::scaleMatrix(float x, float y, float z)
{
	matrix[0][0] = x;
	matrix[1][1] = y;
	matrix[2][2] = z;
}

void Matrix4x4::translateMatrix( float x,  float y, float z)
{
	matrix[3][0] = x;
	matrix[3][1] = y;
	matrix[3][2] = z;
}

void Matrix4x4::orthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	float r_l = right - left;
	if (r_l == 0) return;

	float t_b = top - bottom;
	if (t_b == 0) return;

	float f_n = zFar - zNear;
	if (f_n == 0) return;

	matrix[0][0] = 2 / r_l;
	matrix[1][1] = 2 / t_b;
	matrix[2][2] = -2 / f_n;

	matrix[0][3] = -(right + left) / r_l;
	matrix[1][3] = -(top + bottom) / t_b;
	matrix[2][3] = -(zFar + zNear) / f_n;

}

void Matrix4x4::perspectiveProjection( float fovy, float aspec, float zNear, float zFar)
{	
	float toRadians = fovy*pi/180;
	float f = 1/std::tan(toRadians/2);
	matrix[0][0] = f / aspec;
	matrix[1][1] = f;
	float value = zNear - zFar;
	if (value == 0) return;
	matrix[2][2] = (zFar + zNear) / value;
	matrix[3][2] = (2 * zFar * zNear) / value;
	matrix[2][3] = -1;
	matrix[3][3] = 0;
}

void Matrix4x4::xAxisRotationDegrees( float degrees)
{
	float value = degrees * pi / 180;
	matrix[1][1] = std::cos(value); matrix[2][1] = -std::sin(value);
	matrix[1][2] = std::sin(value); matrix[2][2] =  std::cos(value);
}

void Matrix4x4::xAxisRotationRadians( float radians)
{
	matrix[1][1] = std::cos(radians); matrix[2][1] = -std::sin(radians);
	matrix[1][2] = std::sin(radians); matrix[2][2] =  std::cos(radians);
}

void Matrix4x4::yAxisRotationDegrees( float degrees)
{
	float value = degrees * pi / 180;
	matrix[0][0] = std::cos(value); matrix[0][2] = std::sin(value);
	matrix[2][0] = -std::sin(value); matrix[2][2] = std::cos(value);
}

void Matrix4x4::yAxisRotationRadians( float radians)
{
	matrix[0][0] = std::cos(radians); matrix[0][2] = std::sin(radians);
	matrix[2][0] = -std::sin(radians); matrix[2][2] = std::cos(radians);
}

void Matrix4x4::zAxisRotationDegrees( float degrees)
{
	float value = degrees * pi / 180;
	matrix[0][0] = std::cos(value); matrix[0][1] = -std::sin(value);
	matrix[1][0] = std::sin(value); matrix[1][1] = std::cos(value);
}

void Matrix4x4::zAxisRotationRadians( float radians)
{
	matrix[0][0] = std::cos(radians); matrix[0][1] = -std::sin(radians);
	matrix[1][0] = std::sin(radians); matrix[1][1] = std::cos(radians);
}

void Matrix4x4::lookAtMatrix(const Vector3& eye, const Vector3& center, Vector3 up)
{
	Vector3 f = Vector3(center - eye);
	f.normalizeVector();
	Vector3 s = cross(f, up);
	s.normalizeVector();
	Vector3 u = cross( s, f);

	matrix[0][0] = s.x;
	matrix[1][0] = s.y;
	matrix[2][0] = s.z;

	matrix[0][1] = u.x;
	matrix[1][1] = u.y;
	matrix[2][1] = u.z;

	matrix[0][2] = -f.x;
	matrix[1][2] = -f.y;
	matrix[2][2] = -f.z;

	matrix[3][0] = -dot(s, eye);
	matrix[3][1] = -dot(u, eye);
	matrix[3][2] = dot(f, eye);

}
