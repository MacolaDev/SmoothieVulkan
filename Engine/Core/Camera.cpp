#include "Camera.h"
using namespace Smoothie;
using namespace SmoothieMath;

SmoothieMath::Vector3 Smoothie::Camera::getCameraPosition() const
{
	return cameraPos;
}

void Smoothie::Camera::setCameraPosition(const SmoothieMath::Vector3& position)
{
	cameraPos = position;
}

SmoothieMath::Vector3 Smoothie::Camera::getCameraFront() const
{
	return cameraFront;
}

void Smoothie::Camera::setCameraFront(const SmoothieMath::Vector3& front)
{
	cameraFront = front;
}

SmoothieMath::Vector3 Smoothie::Camera::getCameraUp() const
{
	return cameraUp;
}

void Smoothie::Camera::setCameraUp(const SmoothieMath::Vector3& up)
{
	cameraUp = up;
}

void Camera::updateCameraPosition(const Vector3& position)
{
	cameraPos = position;
}

void Camera::updateCameraFront(const Vector3& front)
{
	cameraFront = front;
}

void Camera::updateCameraMatrices()
{
	cameraMatrix.lookAtMatrix(cameraPos, cameraPos + cameraFront, cameraUp);
	projectionViewMatrix = cameraMatrix * projectionMatrix;
}

void Smoothie::Camera::updateProjectionMatrix(float fovy, float aspec, float zNear, float zFar)
{
	projectionMatrix.perspectiveProjection(fovy, aspec, zNear, zFar);
}

void Smoothie::Camera::setTargetExposure(float exposure)
{
	this->targetExposure = exposure;
}

float Smoothie::Camera::getTargetExposure()
{
	return targetExposure;
}

float Smoothie::Camera::getFarPlane() const
{
	return zFar;
}

CameraUniformBufferData Smoothie::Camera::getUniformBufferData() const
{
	CameraUniformBufferData data;
	data.projectionMatrix = projectionMatrix;
	data.cameraMatrix = cameraMatrix;
	data.cameraPos = cameraPos;
	data.projectionViewMatrix = projectionViewMatrix;
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
	projectionViewMatrix = cameraMatrix * projectionMatrix;

}
