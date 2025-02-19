#pragma once
#include "Smoothie.h"
#include <GLFW/glfw3.h>
struct FreeCamera 
{
	static Smoothie::Camera freeCamera;

	static void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	static void KeyboardCallback(GLFWwindow* window);
	static void updateTime();
	static void resolutionUpdate(int width, int height);
};

