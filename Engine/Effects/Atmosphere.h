#pragma once
#include "Math/SmoothieMath.h"


struct AtmosphereDescriptorSet;
namespace Smoothie 
{
	class Atmosphere
	{
	public:

		void setSunElevation(float angle);

		float getSunElevation() const;

		void setSunRotation(float angle);

		float getSunRotation() const;

		void setSunSize(float size);

		float getSunSize() const;

		void setSunColor(const SmoothieMath::Vector3& color);

		SmoothieMath::Vector3 getSunColor() const;

		void setSunStrength(float strength);

		float getSunStrength() const;

		AtmosphereDescriptorSet getAtmoshpereDescriptorSet() const;

		Atmosphere() = default;
	private:
		friend struct AtmosphereDescriptorSet;


		SmoothieMath::Vector3 sunColor = { 1.0f, 1.0f, 1.0f };
		float sunElevation = 23.0f;
		float sunRotation = 42.0f;

		float sunSize = 0.5f;
		float sunStrength = 3.2f;

	};
}
