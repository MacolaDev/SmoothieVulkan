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

float Vector3::length() const
{
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	return std::sqrt(x2 + y2 + z2);
}

void Vector3::normalizeVector()
{

	float vectorLength = length();

	if (vectorLength == 0) {
		std::cout << "Vector has 0 lenght. Cant normalize it." << std::endl;
		return;
	}

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

	float V1 = A.x; float W1 = B.x;
	float V2 = A.y; float W2 = B.y;
	float V3 = A.z; float W3 = B.z;

	return Vector3(
		V2 * W3 - V3 * W2,
		V3 * W1 - V1 * W3,
		V1 * W2 - V2 * W1
	);
}

