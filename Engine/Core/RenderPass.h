#pragma once
#include <vulkan/vulkan.h>


struct gBufferPass
{
	static VkRenderPass renderPass;
	static void create();
	static void destroy();
};

struct HDRPass
{
	static VkRenderPass renderPass;
	static void create();
	static void destroy();
};

struct PBRPass 
{
	static VkRenderPass renderPass;
	static void create();
	static void destroy();
};

struct BloomPass 
{
	static VkRenderPass renderPass;
	static void create();
	static void destroy();
};