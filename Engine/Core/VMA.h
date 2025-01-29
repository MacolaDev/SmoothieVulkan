#pragma once
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_DETECT_CORRUPTION 1
#include "vk_mem_alloc.h"

//This is only temp solution
class VMA
{
	static VmaAllocator allocator;
public:
	static void createAllocator();
	static void freeAllocator();

	static VmaAllocator getAllocator();

};

