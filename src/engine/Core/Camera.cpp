#include "Camera.h"

using namespace Smoothie;

namespace
{
    struct alignas(16) CameraUniformData
    {
        glm::mat4 projectionMatrix;
        glm::mat4 invProjectionMatrix;
        glm::mat4 cameraMatrix;
        alignas(16) glm::vec3 cameraPos;
        float padding = 0.0f;
        glm::mat4 projectionViewMatrix;
        glm::mat4 invProjectionViewMatrix;
    };
}

void Camera::updateCameraMatrices()
{
    m_ProjectionMatrix = glm::perspective(m_Fov, m_AspecRatio, m_zNear, m_zFar);
	m_InvProjectionMatrix = glm::inverse(m_ProjectionMatrix);

    m_CameraMatrix = glm::lookAt(m_CameraPosition, m_CameraPosition + m_CameraFront, m_CameraUp);

	m_ProjectionViewMatrix = m_ProjectionMatrix * m_CameraMatrix;
	m_InvProjectionViewMatrix = glm::inverse(m_ProjectionViewMatrix);
}

void Camera::getBufferData(std::vector<char> &dataVec) const
{
    auto _data = CameraUniformData();
    _data.projectionMatrix = m_ProjectionMatrix;
    _data.invProjectionMatrix = m_InvProjectionMatrix;
    _data.cameraMatrix = m_CameraMatrix;
    _data.cameraPos = m_CameraPosition;
    _data.projectionViewMatrix = m_ProjectionViewMatrix;
    _data.invProjectionViewMatrix = m_InvProjectionViewMatrix;

    if (dataVec.size() != sizeof(_data))
    {
        dataVec.resize(sizeof(_data), 0);
    }
    std::memcpy(&dataVec[0], &_data, sizeof(_data));
}

VkDeviceSize Camera::getBufferSize() const
{
    return sizeof(CameraUniformData);
}

