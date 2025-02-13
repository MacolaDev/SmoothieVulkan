#pragma once
#include <vulkan/vulkan.h>
#include "Core/Image.h"

struct BRDF
{
	static Image image;
	static void create();
	static void destroy();
private:
	static void createImage();
};

//This is here only temorary until I figure out how do I want to handle this things
class PBRCubemaps
{
public:
	static Image HDRCubemap;
	static Image IrradianceMap;
	static Image PrefilterMap;

	static void create(const std::string& hdriImage);

	static void destroy();
};