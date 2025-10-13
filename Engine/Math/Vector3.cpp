#include "Vector3.h"
#include <iostream>

using namespace SmoothieMath;
void SmoothieMath::Vector3::setX(float value)
{
	x = value;
}

void SmoothieMath::Vector3::setY(float value)
{
	y = value;
}

void SmoothieMath::Vector3::setZ(float value)
{
	z = value;
}

Vector3::Vector3(float x, float y, float z)
{
	(*this).x = x;
	(*this).y = y;
	(*this).z = z;

}

SmoothieMath::Vector3::Vector3(float value)
{
	x = y = z = value;
}

float Vector3::length() const
{
	const float x2 = x * x;
	const float y2 = y * y;
	const float z2 = z * z;
	return std::sqrt(x2 + y2 + z2);
}

void Vector3::normalizeVector()
{
	const float vectorLength = length();

	x = x / vectorLength;
	y = y / vectorLength;
	z = z / vectorLength;
}

const float* Vector3::dataPointer() const
{
	return  &x;
}

Vector3 Vector3::operator + (const Vector3& other) const
{
	return Vector3(
		x + other.x,
		y + other.y,
		z + other.z
	);
}

Vector3 Vector3::operator * (const Vector3& other) const
{
	return Vector3(
		x * other.x,
		y * other.y,
		z * other.z);
}

Vector3 Vector3::operator * ( const float& scalar) const
{
	return Vector3(
		x * scalar,
		y * scalar,
		z * scalar);
}

Vector3 SmoothieMath::Vector3::operator/(float& scalar) const
{
	return Vector3(x/scalar, y/scalar, z/scalar);
}

Vector3 SmoothieMath::Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}

void SmoothieMath::Vector3::operator+=(const Vector3& other)
{
	x = x + other.x;
	y = y + other.y;
	z = z + other.z;
}

void SmoothieMath::Vector3::operator-=(const Vector3& other)
{
	x = x - other.x;
	y = y - other.y;
	z = z - other.z;
}

Vector3 SmoothieMath::Vector3::cross(const Vector3& other) const
{	
	float V1 = x; float W1 = other.x;
	float V2 = y; float W2 = other.y;
	float V3 = z; float W3 = other.z;

	return Vector3(
		V2 * W3 - V3 * W2, 
		V3 * W1 - V1 * W3,
		V1 * W2 - V2 * W1
	);
}

Vector3 Vector3::operator - (const Vector3& other) const
{
	return Vector3(
		x - other.x,
		y - other.y,
		z - other.z
	);
}

void Vector3::print() const
{
	std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
}

float SmoothieMath::dot(const Vector3& vector, const Vector3& vector2)
{
	return (vector.x * vector2.x) + (vector.y * vector2.y) + (vector.z * vector2.z);
}

Vector3 SmoothieMath::normalize(Vector3& vector)
{	
	return vector * (1/ vector.length());
}

Vector3 SmoothieMath::cross(const Vector3& A, const Vector3& B)
{

	const float& V1 = A.x; const float& W1 = B.x;
	const float& V2 = A.y; const float& W2 = B.y;
	const float& V3 = A.z; const float& W3 = B.z;

	return Vector3(
		V2 * W3 - V3 * W2,
		V3 * W1 - V1 * W3,
		V1 * W2 - V2 * W1
	);
}

Vector3 SmoothieMath::operator*(float scalar, const Vector3& other)
{
	return other * scalar;
}
