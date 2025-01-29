#pragma once
#include "Math/SmoothieMath.h"


using namespace SmoothieMath;
class SmoothieCore;
class AutomaticExposure;

namespace Smoothie 
{

	class Camera
	{
		Matrix4x4 cameraMatrix;
		Matrix4x4 projectionMatrix;
		Matrix4x4 projectionViewMatrix;

		float zNear, zFar, fovy, aspec;
		float targetExposure;
		float minExposure, avrExposure, maxExposure;

		friend class SmoothieCore;
		friend class AutomaticExposure;
	

	public:

		Vector3 cameraPos, cameraFront, cameraUp;

		void updateCameraPosition(const Vector3& position);
		void updateCameraFront(const Vector3& front);

		void updateCameraMatrix();
		void updateProjectionMatrix(float fovy, float aspec, float zNear, float zFar);
		float* matrixPtr();

		Camera(const Vector3& cameraPos, const Vector3& cameraFront, const Vector3& cameraUp, float fovy, float aspec, float zNear, float zFar);
		Camera();

		void setTargetExposure(float exposure);
		float getTargetExposure();

		float getFarPlane() const;
	};

}