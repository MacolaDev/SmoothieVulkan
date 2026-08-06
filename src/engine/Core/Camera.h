#pragma once
#include "../Common.h"

namespace Smoothie 
{
	class Camera
	{
	public:

        virtual void updateCameraMatrices();
	    virtual void getBufferData(std::vector<char>& dataVec) const;
        virtual VkDeviceSize getBufferSize() const;

	    inline const glm::mat4& getCameraMatrix() const { return m_CameraMatrix; }
	    inline void setCameraMatrix(const glm::mat4& matrix) { m_CameraMatrix = matrix; }

	    inline const glm::mat4& getProjectionMatrix() const { return m_ProjectionMatrix; }
	    inline void setProjectionMatrix(const glm::mat4& matrix) { m_ProjectionMatrix = matrix; }

	    inline const glm::vec3& getCameraPosition() const { return m_CameraPosition; }
	    inline void setCameraPosition(const glm::vec3& position) { m_CameraPosition = position; }

	    inline const glm::vec3& getCameraFront() const { return m_CameraFront; }
	    inline void setCameraFront(const glm::vec3& front) { m_CameraFront = front; }

	    inline const glm::vec3& getCameraUp() const { return m_CameraUp; };
	    inline void setCameraUp(const glm::vec3& up) { m_CameraUp = up; }

	    inline float getZNear() const { return m_zNear; }
	    inline void setZNear(float zNear) { m_zNear = zNear; }

	    inline float getZFar() const { return m_zFar; }
	    inline void setZFar(float zFar) { m_zFar = zFar; }

	    inline float getFov() const { return m_Fov; }
	    inline void setFov(float fov) { m_Fov = fov; }

	    inline float getAspectRatio() const { return m_AspecRatio; }
	    inline void setAspectRatio(float aspec) { m_AspecRatio = aspec; }

	    virtual ~Camera() = default;
	protected:
	    glm::mat4 m_CameraMatrix;
		glm::mat4 m_ProjectionMatrix;

		glm::mat4 m_ProjectionViewMatrix;
		glm::mat4 m_InvProjectionMatrix;
		glm::mat4 m_InvProjectionViewMatrix;

	    glm::vec3 m_CameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	    glm::vec3 m_CameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	    glm::vec3 m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	    float m_zNear = 0.01f;
	    float m_zFar = 1000.0f;
	    float m_Fov = 45.0f;
	    float m_AspecRatio = 1.0f;

	};

}
