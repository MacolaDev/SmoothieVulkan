#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VMA.h"
#include <memory>

enum VertexBufferType {
	XYZNUVTB = 0,
	XYZNUVTBC = 1
};

class VertexBufferBase
{
public:
	virtual std::vector<VkVertexInputAttributeDescription> getVertexAttributes() const;
	virtual void populateBindingDescription(VkVertexInputBindingDescription& bindingDescription) const;
};

struct xyznuvtb : public VertexBufferBase
{
	float x, y, z;
	unsigned int n;
	float u, v;
	unsigned int t, b;

	std::vector<VkVertexInputAttributeDescription> getVertexAttributes() const override;
	void populateBindingDescription(VkVertexInputBindingDescription& bindingDescription) const override;
};

struct xyznuvtbc : public VertexBufferBase
{
	float x, y, z;
	unsigned int n;
	float u, v;
	unsigned int t, b, color;

	std::vector<VkVertexInputAttributeDescription> getVertexAttributes() const override;
	void populateBindingDescription(VkVertexInputBindingDescription& bindingDescription) const override;
};
