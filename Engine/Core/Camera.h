#pragma once
#include "Math/SmoothieMath.h"
#include <vulkan/vulkan.h>

class SmoothieCore;
class AutomaticExposure;

struct CameraUniformBufferData 
{
	SmoothieMath::Matrix4x4 projectionMatrix;
	SmoothieMath::Matrix4x4 cameraMatrix;
	SmoothieMath::Vector3 cameraPos;
	int padding = 0;
	SmoothieMath::Matrix4x4 projectionViewMatrix;
};

namespace Smoothie 
{
	class Camera
	{
	public:
		SmoothieMath::Vector3 getCameraPosition() const;

		void setCameraPosition(const SmoothieMath::Vector3& position);

		SmoothieMath::Vector3 getCameraFront() const;

		void setCameraFront(const SmoothieMath::Vector3& front);

		SmoothieMath::Vector3 getCameraUp() const;

		void setCameraUp(const SmoothieMath::Vector3& up);

		void updateCameraPosition(const SmoothieMath::Vector3& position);

		void updateCameraFront(const SmoothieMath::Vector3& front);

		void updateCameraViewMatrices();

		void updateProjectionMatrix(float fovy, float aspec, float zNear, float zFar);
		
		void updateProjectionMatrix();

		void setTargetExposure(float exposure);

		void setAspecRatio(float ratio);

		float getTargetExposure();

		float getFarPlane() const;

		CameraUniformBufferData getUniformBufferData() const;

		Camera(
			const SmoothieMath::Vector3& cameraPos,
			const SmoothieMath::Vector3& cameraFront,
			const SmoothieMath::Vector3& cameraUp,
			float fovy, float aspec, float zNear, float zFar);
		Camera() = default;


	private:
		SmoothieMath::Matrix4x4 cameraMatrix;
		SmoothieMath::Matrix4x4 projectionMatrix;
		SmoothieMath::Matrix4x4 projectionViewMatrix;

		SmoothieMath::Vector3 cameraPos = { 0, 0, 0 }, cameraFront = { 1, 1, 1 }, cameraUp = { 0, 1, 0 };

		float zNear = 0.1f, zFar = 1000.0f, fovy = 45.0f, aspec = 1280/720;
		float targetExposure = 0.5f;
		float minExposure = 0.0f, avrExposure = 0.5f, maxExposure = 1.0f;

		friend class SmoothieCore;
		friend class AutomaticExposure;
	

	};

}
