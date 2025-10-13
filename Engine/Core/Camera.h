#pragma once
#include "Math/SmoothieMath.h"
#include <vulkan/vulkan.h>

namespace Smoothie 
{
	//Main descriptor set data
	struct CameraUniformBufferData
	{
		alignas(16) SmoothieMath::Matrix4x4 projectionMatrix;
		alignas(16) SmoothieMath::Matrix4x4 invProjectionMatrix;
		alignas(16) SmoothieMath::Matrix4x4 cameraMatrix;
		alignas(16) SmoothieMath::Vector3 cameraPos;
		alignas(16) SmoothieMath::Matrix4x4 projectionViewMatrix;
		alignas(16) SmoothieMath::Matrix4x4 invProjectionViewMatrix;
		unsigned int SCR_WIDTH = 0, SCR_HEIGHT = 0;
	};

	//Main camera class for engine to use;
	class Camera
	{

	protected:
		SmoothieMath::Matrix4x4 cameraMatrix;
		SmoothieMath::Matrix4x4 projectionMatrix;
		SmoothieMath::Matrix4x4 projectionViewMatrix;
		SmoothieMath::Matrix4x4 invProjectionMatrix;
		SmoothieMath::Matrix4x4 invProjectionViewMatrix;

		SmoothieMath::Vector3 cameraPos = { 0, 0, 0 }, cameraFront = { 1, 1, 1 }, cameraUp = { 0, 1, 0 };
		float zNear = 0.1f, zFar = 1000.0f, fovy = 45.0f, aspec = 1280 / 720;

	public:
		inline SmoothieMath::Vector3 getCameraPosition() const { return cameraPos; }
		inline void setCameraPosition(const SmoothieMath::Vector3& position) { this->cameraPos = position; }

		inline SmoothieMath::Vector3 getCameraFront() const { return cameraFront; }
		inline void setCameraFront(const SmoothieMath::Vector3& front) { this->cameraFront = front; }
		
		inline SmoothieMath::Vector3 getCameraUp() const { return cameraUp; };
		inline void setCameraUp(const SmoothieMath::Vector3& up) { this->cameraUp = up; }

		inline float getZNear() const { return zNear; }
		inline void setZNear(float zNear) { this->zNear = zNear; }

		inline float getZFar() const { return zFar; }
		inline void setZFar(float zFar) { this->zFar = zFar; }

		inline float getFov() const { return fovy; }
		inline void setFov(float fov) { this->fovy = fov; }

		inline float getAspectRatio() const { return aspec; }
		inline void setAspectRatio(float aspec) { this->aspec = aspec; }

		void updateCameraMatrices();

		CameraUniformBufferData getCameraBufferData() const;

		Camera(
			const SmoothieMath::Vector3& cameraPos,
			const SmoothieMath::Vector3& cameraFront,
			const SmoothieMath::Vector3& cameraUp,
			float fovy, float aspec, float zNear, float zFar);
		Camera() = default;

	};

}
