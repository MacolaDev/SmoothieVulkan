#pragma once
#include "Vector4.h"
#include "Vector3.h"
#include "Matrix3x3.h"

namespace SmoothieMath {
	struct Matrix3x3;
	struct Matrix4x4
	{
		Vector4 column0 = { 1.0f, 0.0f, 0.0f, 0.0f };
		Vector4 column1 = { 0.0f, 1.0f, 0.0f, 0.0f };
		Vector4 column2 = { 0.0f, 0.0f, 1.0f, 0.0f };
		Vector4 column3 = { 0.0f, 0.0f, 0.0f, 1.0f };

		Matrix4x4() = default;
		Matrix4x4(const Vector4& row0, const Vector4& row1, const Vector4& row2, const Vector4& row3);
		Matrix4x4(const Matrix3x3& matrix);

		void scaleMatrix( float x,  float y,  float z);
		void translateMatrix( float x,  float y,float z);

		void orthoProjection(float left, float right, float bottom, float top, float zNear, float zFar);
		
		//Degrees
		void perspectiveProjection(float fovy, float aspec, float zNear,  float zFar);

		/*void xAxisRotationDegrees( float degrees);
		void xAxisRotationRadians( float radians);

		void yAxisRotationDegrees( float degrees);
		void yAxisRotationRadians( float radians);

		void zAxisRotationDegrees( float degrees);
		void zAxisRotationRadians( float radians);*/

		void lookAtMatrix(const Vector3& eye, const Vector3& center,  const Vector3& up);

		float* dataPointer();
	
		//void inverse();
		
		//Returns normal matrix out of model matrix
		SmoothieMath::Matrix3x3 normalMatrix() const;
		Matrix3x3 getRotationMatrix() const;

		//SmoothieMath::Vector3 getScaleComponent() const;

		//Returns yaw, pitch, roll (rotation around X, Y and Z components respectuffuly) in degrees
		//SmoothieMath::Vector3 getEulerAnglesComponent() const;

		//SmoothieMath::Vector3 getPositionComponent() const;

		//Takes XYZ position, XYZ euler angles in degrees and XYZ scale to generate transformation matrix 
		void transformMatrix(
			const SmoothieMath::Vector3& position, 
			const SmoothieMath::Vector3& rotationEuler,
			const SmoothieMath::Vector3& scale);

		void transformMatrix(
			const SmoothieMath::Vector3& position,
			const SmoothieMath::Matrix3x3& rotationMatrix,
			const SmoothieMath::Vector3& scale);

		//Matrix4x4 operator * (float scalar);

		friend Matrix4x4 operator * (const Matrix4x4& A, const Matrix4x4& B);
		friend Vector4 operator * (const Matrix4x4& matrix, const Vector4& vector);
		friend Matrix4x4 inverse(const Matrix4x4& matrix);
		friend void transpose(Matrix4x4& matrix);
	};

	void transpose(Matrix4x4& matrix);
	Vector4 operator * (const Matrix4x4& matrix, const Vector4& vector);
	Matrix4x4 operator * (const Matrix4x4& A, const Matrix4x4& B);
	Matrix4x4 operator * (float scalar, const Matrix4x4& matrix);
	Matrix4x4 inverse(const Matrix4x4& matrix);
}