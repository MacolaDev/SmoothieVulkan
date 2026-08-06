#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <queue>
#include <memory>
#include <cassert>
#include <cmath>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <type_traits>

#include "vulkan/vulkan.h"

#define SMOOTHIE_STRINGIFY(x) #x
#define SMOOTHIE_TOSTRING(x) SMOOTHIE_STRINGIFY(x)
#define SMOOTHIE_LOCATION __FILE__ "(" SMOOTHIE_TOSTRING(__LINE__) "):"
#define SMOOTHIE_SRC_LOCATION(msg) __FILE__ "(" SMOOTHIE_TOSTRING(__LINE__) "): " msg
#define SMOOTHIE_VERBOSE_WITH_SRC(msg) SmoothieCore::logVerbose(SMOOTHIE_SRC_LOCATION(msg))
#define SMOOTHIE_ERROR_WITH_SRC(msg) SmoothieCore::logError(SMOOTHIE_SRC_LOCATION(msg))
#define SMOOTHIE_CRITICAL_WITH_SRC(msg) SmoothieCore::logError(SMOOTHIE_SRC_LOCATION(msg))

#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "fmt/std.h"

#define SMOOTHIE_VK_HANDLE_FORMATTER(Type)                                                                                                  \
namespace fmt{template <> struct formatter<Type>{                                                                                      \
constexpr auto parse(fmt::format_parse_context& ctx){return ctx.begin();}                                                                   \
template <typename FormatContext>auto format(const Type& p, FormatContext& ctx) const{return fmt::format_to(ctx.out(), "{}", fmt::ptr(p));} \
};}                                                                                                                                         \

SMOOTHIE_VK_HANDLE_FORMATTER(VkImage)
SMOOTHIE_VK_HANDLE_FORMATTER(VkImageView)
SMOOTHIE_VK_HANDLE_FORMATTER(VkFramebuffer)
SMOOTHIE_VK_HANDLE_FORMATTER(VkCommandPool)
SMOOTHIE_VK_HANDLE_FORMATTER(VkCommandBuffer)
SMOOTHIE_VK_HANDLE_FORMATTER(VkSemaphore)
SMOOTHIE_VK_HANDLE_FORMATTER(VkFence)

//TODO: Add more handles

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_ALIGNED_GENTYPES
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "vk_mem_alloc.h"


