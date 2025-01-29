#pragma once
namespace SmoothieMath {
	struct Vector4
	{
		float x, y, z, w;

		Vector4(float x = 0, float y = 0, float z = 0, float w = 0);
		Vector4(const float* data);

		float length() const;

		void normalizeVector();

		float* dataPointer();

		Vector4 operator + (const Vector4& other) const;
		Vector4 operator - (const Vector4& other) const;
		Vector4 operator * (const Vector4& other) const;
		Vector4 operator * (const float& scalar) const;
		float operator[] (const int index) const;
		void print() const;
	};

	float dot(const Vector4& vector, const Vector4& vector2);
}