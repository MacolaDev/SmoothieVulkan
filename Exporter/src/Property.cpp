#include "Property.h"
#include "imgui.h"
#include <iostream>
#include <sstream>

using namespace Smoothie::XML;

static Element createElement(const std::string& propertyType, const std::string& propertyName, const std::string& propertyValue)
{
	Element textureElement(propertyType, propertyName);
	textureElement.children.push_back({ "value",  propertyValue });
	return textureElement;
}

int getRelativeFromFullPath(const std::string& fullPath, std::string& relative_path, const std::string& key)
{
	size_t pos = fullPath.find(key);
	if (pos != std::string::npos) {
		relative_path = fullPath.substr(pos);
		return 0;
	}
	return -1;
}

void PropertyFloat::draw()
{
	ImGui::DragFloat(propertyName.c_str(), &value);
}

void PropertyFloat::set_selected_option(const Smoothie::XML::Element& propertiesElement)
{
	for (size_t i = 0; i < propertiesElement.children.size(); i++)
	{
		const auto& element = propertiesElement.children[i];
		if ((element.name == "Float") && (element.textContent == propertyName) && (element.hasChild("value")))
		{
			const auto& elm_value = element.getChild("value");
			value = elm_value.getFloat();
		}
	}

}

void PropertyFloat::save_selected_option(Smoothie::XML::Element& propertiesElement) const
{
	return propertiesElement.children.push_back(createElement("Float", propertyName, std::to_string(value)));
}

Smoothie::XML::Element PropertyFloat::as_xml_element() const
{
	return createElement("Float", propertyName, std::to_string(value));
}

void PropertyVector3::draw()
{
	ImGui::DragFloat3(propertyName.c_str(), value);
}

void PropertyVector3::set_selected_option(const Smoothie::XML::Element& propertiesElement)
{
	for (size_t i = 0; i < propertiesElement.children.size(); i++)
	{
		const auto& element = propertiesElement.children[i];
		if ((element.name == "Vector3") && (element.textContent == propertyName) && (element.hasChild("value")))
		{
			const auto& elm_value = element.getChild("value");
			const auto& vector = elm_value.getVector3();
			value[0] = vector.x;
			value[1] = vector.y;
			value[2] = vector.y;
		}
	}
}

void PropertyVector3::save_selected_option(Smoothie::XML::Element& propertiesElement) const
{
	const std::string result = 
		std::to_string(value[0]) + " " + 
		std::to_string(value[1]) + " " +
		std::to_string(value[2]);
	propertiesElement.children.push_back(createElement("Vector3", propertyName, result.c_str()));
}

Smoothie::XML::Element PropertyVector3::as_xml_element() const
{
	const std::string result =
		std::to_string(value[0]) + " " +
		std::to_string(value[1]) + " " +
		std::to_string(value[2]);
	return createElement("Vector3", propertyName, result.c_str());
}

void PropertyVector4::draw()
{
	ImGui::DragFloat4(propertyName.c_str(), value);
}

void PropertyVector4::set_selected_option(const Smoothie::XML::Element& propertiesElement)
{
	for (size_t i = 0; i < propertiesElement.children.size(); i++)
	{
		const auto& element = propertiesElement.children[i];
		if ((element.name == "Vector4") && (element.textContent == propertyName) && (element.hasChild("value")))
		{
			const auto& elm_value = element.getChild("value");
			const auto& vector = elm_value.getVector4();
			value[0] = vector.x;
			value[1] = vector.y;
			value[2] = vector.z;
			value[3] = vector.w;
		}
	}
}

void PropertyVector4::save_selected_option(Smoothie::XML::Element& propertiesElement) const
{
	const std::string result =
		std::to_string(value[0]) + " " +
		std::to_string(value[1]) + " " +
		std::to_string(value[2]) + " " +
		std::to_string(value[3]);
	propertiesElement.children.push_back(createElement("Vector4", propertyName, result.c_str()));
}

Smoothie::XML::Element PropertyVector4::as_xml_element() const
{
	const std::string result =
		std::to_string(value[0]) + " " +
		std::to_string(value[1]) + " " +
		std::to_string(value[2]) + " " +
		std::to_string(value[3]);
	return createElement("Vector4", propertyName, result.c_str());
}

void PropertyVector2::draw()
{
	ImGui::DragFloat2(propertyName.c_str(), value);
}

void PropertyVector2::set_selected_option(const Smoothie::XML::Element& propertiesElement)
{
	for (size_t i = 0; i < propertiesElement.children.size(); i++)
	{
		const auto& element = propertiesElement.children[i];
		if ((element.name == "Vector2") && (element.textContent == propertyName) && (element.hasChild("value")))
		{
			const auto& elm_value = element.getChild("value");
			const auto& vector = elm_value.getVector2();
			value[0] = vector.x;
			value[1] = vector.y;
		}
	}
}

void PropertyVector2::save_selected_option(Smoothie::XML::Element& propertiesElement) const
{
	const std::string result = std::to_string(value[0]) + " " + std::to_string(value[1]);
	propertiesElement.children.push_back(createElement("Vector2", propertyName, result.c_str()));
}

Smoothie::XML::Element PropertyVector2::as_xml_element() const
{
	const std::string result = std::to_string(value[0]) + " " + std::to_string(value[1]);
	return createElement("Vector2", propertyName, result.c_str());
}

void PropertyUnsignedInt::draw()
{
	constexpr unsigned int max_value = UINT32_MAX;
	ImGui::DragScalar(propertyName.c_str(), ImGuiDataType_U32, &value, 1.0f, 0, &max_value, "%d");
}

void PropertyUnsignedInt::set_selected_option(const Smoothie::XML::Element& propertiesElement)
{
	for (size_t i = 0; i < propertiesElement.children.size(); i++)
	{
		const auto& element = propertiesElement.children[i];
		if ((element.name == "UInt") && (element.textContent == propertyName) && (element.hasChild("value")))
		{
			const auto& elm_value = element.getChild("value");
			value = static_cast<unsigned int>(elm_value.getInt());
		}
	}
}

void PropertyUnsignedInt::save_selected_option(Smoothie::XML::Element& propertiesElement) const
{
	propertiesElement.children.push_back(createElement("UInt", propertyName, std::to_string(value)));
}

Smoothie::XML::Element PropertyUnsignedInt::as_xml_element() const
{
	return createElement("UInt", propertyName, std::to_string(value));
}

void PropertySampler2D::draw()
{
	ImGui::Combo(propertyName.c_str(), &currentIndex, data.data(), static_cast<int>(data.size()));
}

Smoothie::XML::Element PropertySampler2D::as_xml_element() const
{
	std::string relative_path;
	if (getRelativeFromFullPath(data[currentIndex], relative_path, "resources") != 0)
	{
		std::cout << "ERROR: PropertySampler2D: Can't get a relative path from a " << data[currentIndex] << std::endl;
	}
	return createElement("Texture2D", propertyName, relative_path);
}

void PropertySampler2D::set_selected_option(const Smoothie::XML::Element& propertiesElement)
{
	for (size_t i = 0; i < propertiesElement.children.size(); i++)
	{
		const auto& element = propertiesElement.children[i];
		if ((element.name == "Texture2D") && (element.textContent == propertyName) && (element.hasChild("value")))
		{
			const auto& value = element.getChild("value");
			for (size_t index = 0; index < data.size(); index++)
			{
				if (value.textContent == data[index])
				{
					currentIndex = index;
					return;
				}
			}
		}

	}

}

void PropertySampler2D::save_selected_option(Smoothie::XML::Element& propertiesElement) const
{
	propertiesElement.children.push_back(createElement("Texture2D", propertyName, data[currentIndex]));
}

Smoothie::XML::Element PropertyBase::as_xml_element() const
{
	return Smoothie::XML::Element();
}

void PropertyBase::set_selected_option(const Smoothie::XML::Element& propertiesElement)
{

}

void PropertyBase::save_selected_option(Smoothie::XML::Element& propertiesElement) const
{

}
