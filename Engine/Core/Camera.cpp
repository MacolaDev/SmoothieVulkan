#include "Camera.h"
#include <math.h>

using namespace Smoothie;
using namespace SmoothieMath;

void Camera::updateCameraMatrices()
{
	projectionMatrix.perspectiveProjection(fovy, aspec, zNear, zFar);
	invProjectionMatrix = inverse(projectionMatrix);
	cameraMatrix.lookAtMatrix(cameraPos, cameraPos + cameraFront, cameraUp);
	projectionViewMatrix = projectionMatrix * cameraMatrix;
	invProjectionViewMatrix = inverse(projectionViewMatrix);
}

CameraUniformBufferData Smoothie::Camera::getCameraBufferData() const
{
	CameraUniformBufferData data;
	data.projectionMatrix = projectionMatrix;
	data.cameraMatrix = cameraMatrix;
	data.cameraPos = cameraPos;
	data.projectionViewMatrix = projectionViewMatrix;
	data.invProjectionViewMatrix = invProjectionViewMatrix;
	return data;
}

Camera::Camera(const Vector3& cameraPos, const Vector3& cameraFront, const Vector3& cameraUp, float fovy, float aspec, float zNear, float zFar)
{
	this->cameraPos = cameraPos;
	this->cameraFront = cameraFront;
	this->cameraUp = cameraUp;

	this->zFar = zFar;
	this->zNear = zNear;

	this->fovy = fovy;
	this->aspec = aspec;
	
	cameraMatrix.lookAtMatrix(cameraPos, cameraPos + cameraFront, cameraUp);
	projectionMatrix.perspectiveProjection(fovy, aspec, zNear, zFar);
	projectionViewMatrix = projectionMatrix * cameraMatrix;
	invProjectionMatrix = inverse(projectionMatrix);
	invProjectionViewMatrix = inverse(projectionViewMatrix);
}

static inline Vector3 computeWorldSpacePosition(const SmoothieMath::Matrix4x4& invProjectionViewMatrix, const Vector3& NDC) 
{
	const Vector4 result = invProjectionViewMatrix * Vector4(NDC.x, NDC.y, NDC.z, 1.0f);
	return { 
		result.x / result.w, 
		result.y / result.w, 
		result.z / result.w};
}

static inline Vector3 getNormalFromTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
	const auto a = v1 - v0;
	const auto b = v2 - v0;
	return cross(a, b);
}
