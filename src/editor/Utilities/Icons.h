// #pragma once
// #include "Common.h"
//
// namespace SmoothieEditor
// {
// 	struct Icon
// 	{
// 		const std::string path;
// 		VkSampler sampler = nullptr;
// 	public:
// 		explicit Icon(std::string path, VkSampler sampler): path(std::move(path)), sampler(sampler) {};
//
// 		VkDescriptorSet TextureID = nullptr;
// 		VkImage image = nullptr;
// 		VkImageView imageView = nullptr;
// 		VmaAllocation allocation = nullptr;
//
// 		int create();
// 		void destroy();
// 	};
//
// }
//
