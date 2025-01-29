#pragma once
namespace SmoothieMath {
	struct Vector3
	{
		float x, y, z;

		void setX(float value);
		void setY(float value);
		void setZ(float value);


		Vector3(float x = 0, float y = 0, float z = 0);
		
		float length() const;

		void normalizeVector();

		const float* dataPointer() const;

		Vector3 operator + (const Vector3& other) const;
		Vector3 operator - (const Vector3& other) const;
		Vector3 operator * (const Vector3& other) const;
		Vector3 operator * (const float& scalar) const;
		Vector3 operator / (float& scalar) const;

		//Returns A = A x other
		Vector3 cross(const Vector3& other) const;
		void print() const;
	};


	float dot(const Vector3& vector, const Vector3& vector2);
	Vector3 normalize(Vector3& vector);
	Vector3 cross(const Vector3& A, const Vector3& B);
	
	
}
