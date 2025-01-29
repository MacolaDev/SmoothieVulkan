#include "Camera.h"
using namespace Smoothie;

void Camera::updateCameraPosition(const Vector3& position)
{
	cameraPos = position;
}

void Camera::updateCameraFront(const Vector3& front)
{
	cameraFront = front;
}

void Camera::updateCameraMatrix()
{
	cameraMatrix.lookAtMatrix(cameraPos, cameraPos + cameraFront, cameraUp);
	projectionViewMatrix = cameraMatrix * projectionMatrix;
}

void Smoothie::Camera::updateProjectionMatrix(float fovy, float aspec, float zNear, float zFar)
{
	projectionMatrix.perspectiveProjection(fovy, aspec, zNear, zFar);
}

float* Camera::matrixPtr()
{
	return cameraMatrix.dataPointer();
}

Camera::Camera() :cameraPos(0, 0, 0), cameraFront(1, 1, 1), cameraUp(0, 1, 0), zFar(1000.0f), zNear(0.1f), 
targetExposure(0.5f), minExposure(0.0f), avrExposure(0.5f), maxExposure(1.0f)	
{
	cameraMatrix = Matrix4x4();
	projectionMatrix = Matrix4x4();
	projectionViewMatrix = Matrix4x4();
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

Camera::Camera(const Vector3& cameraPos, const Vector3& cameraFront, const Vector3& cameraUp, float fovy, float aspec, float zNear, float zFar) :
cameraPos(0, 0, 0), cameraFront(1, 1, 1), cameraUp(0, 1, 0), zFar(1000.0f), zNear(0.1f),
targetExposure(0.5f), minExposure(0.0f), avrExposure(0.5f), maxExposure(1.0f)
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
