//#include "Atmosphere.h"
//#include <cmath>
//using namespace Smoothie;
//using namespace SmoothieMath;
//
//void Smoothie::Atmosphere::setSunElevation(float angle)
//{
//	sunElevation = angle;
//}
//
//float Smoothie::Atmosphere::getSunElevation() const
//{
//	return sunElevation;
//}
//
//void Smoothie::Atmosphere::setSunRotation(float angle)
//{
//	sunRotation = angle;
//}
//
//float Smoothie::Atmosphere::getSunRotation() const
//{
//	return sunRotation;
//}
//
//void Smoothie::Atmosphere::setSunSize(float size)
//{
//	sunSize = size;
//}
//
//float Smoothie::Atmosphere::getSunSize() const
//{
//	return sunSize;
//}
//
//void Smoothie::Atmosphere::setSunColor(const SmoothieMath::Vector3& color)
//{
//	sunColor = color;
//}
//
//SmoothieMath::Vector3 Smoothie::Atmosphere::getSunColor() const
//{
//	return sunColor;
//}
//
//void Smoothie::Atmosphere::setSunStrength(float strength)
//{
//	sunStrength = strength;
//}
//
//float Smoothie::Atmosphere::getSunStrength() const
//{
//	return sunStrength;
//}
//
//AtmosphereDescriptorSet Smoothie::Atmosphere::getAtmoshpereDescriptorSet() const
//{
//	AtmosphereDescriptorSet descriptorSet;
//
//	Vector3 sunPosition;
//	
//	sunPosition.x = sinf((90.0f - sunElevation) * pi / 180.0f) * cosf(sunRotation * pi / 180.0f);
//	sunPosition.y = sinf((90.0f - sunElevation) * pi / 180.0f) * sinf(sunRotation * pi / 180.0f);
//	sunPosition.z = cosf((90.0f - sunElevation) * pi / 180.0f);
//
//	descriptorSet.sunModelMatrix.transformMatrix(
//		sunPosition, 
//		{0, 90.0f - sunElevation, sunRotation}, {1, 1, 1});
//	
//	descriptorSet.sunWorldPosition = sunPosition;
//	descriptorSet.sunStrength = sunStrength;
//	descriptorSet.sunSize = sunSize;
//	descriptorSet.sunColor = sunColor;
//
//	return descriptorSet;
//}
