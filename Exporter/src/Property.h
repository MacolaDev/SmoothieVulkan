#pragma once
#include "ResourceManager/XML.h"
#include <string>
#include <vector>
struct PropertyBase
{
	std::string propertyName;
	
	virtual void draw() {};
	
	//This is used as result of an output 
	virtual Smoothie::XML::Element as_xml_element() const = 0;

	//Set selected option from saved xml file
	virtual void set_selected_option(const Smoothie::XML::Element& propertiesElement) = 0;
	
	//Save selected option to xml tree
	virtual void save_selected_option(Smoothie::XML::Element& propertiesElement) const = 0;

	PropertyBase(const std::string& name) : propertyName(name) {};
};

struct PropertyFloat : public PropertyBase
{
	float value = 0.0f;
	void draw() override;

	void set_selected_option(const Smoothie::XML::Element& propertiesElement) override;
	void save_selected_option(Smoothie::XML::Element& propertiesElement) const override;
	Smoothie::XML::Element as_xml_element() const override;

	PropertyFloat(const std::string& name) : PropertyBase(name) {};
};

struct PropertyVector2 : public PropertyBase
{
	float value[2] = { 0.0f };
	void draw() override;
	void set_selected_option(const Smoothie::XML::Element& propertiesElement) override;
	void save_selected_option(Smoothie::XML::Element& propertiesElement) const override;
	Smoothie::XML::Element as_xml_element() const override;

	PropertyVector2(const std::string& name) : PropertyBase(name) {};
};

struct PropertyVector3 : public PropertyBase
{
	float value[3] = { 0.0f };
	void draw() override;
	void set_selected_option(const Smoothie::XML::Element& propertiesElement) override;
	void save_selected_option(Smoothie::XML::Element& propertiesElement) const override;
	Smoothie::XML::Element as_xml_element() const override;

	PropertyVector3(const std::string& name) : PropertyBase(name) {}; 
};

struct PropertyVector4 : public PropertyBase
{
	float value[4] = { 0.0f };
	void draw() override;
	void set_selected_option(const Smoothie::XML::Element& propertiesElement) override;
	void save_selected_option(Smoothie::XML::Element& propertiesElement) const override;
	Smoothie::XML::Element as_xml_element() const override;

	PropertyVector4(const std::string& name) : PropertyBase(name) {};
};

struct PropertyUnsignedInt : public PropertyBase
{
	unsigned int value = 0;
	void draw() override;
	void set_selected_option(const Smoothie::XML::Element& propertiesElement) override;
	void save_selected_option(Smoothie::XML::Element& propertiesElement) const override;
	Smoothie::XML::Element as_xml_element() const override;
	PropertyUnsignedInt(const std::string& name) : PropertyBase(name) {};
};

struct PropertySampler2D : public PropertyBase
{
	std::vector<const char*> data;
	int currentIndex = 0;
	void draw() override;

	Smoothie::XML::Element as_xml_element() const override;

	void set_selected_option(const Smoothie::XML::Element& propertiesElement) override;
	void save_selected_option(Smoothie::XML::Element& propertiesElement) const override;

	PropertySampler2D(const std::string& name, const std::vector<const char*> textures) : PropertyBase(name), data(textures) {};
};