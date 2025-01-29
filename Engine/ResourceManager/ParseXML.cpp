#include "ParseXML.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace SmoothieMath;

//Element starts with < and ends with > 
std::string ElementToken(std::ifstream& data) {
	std::string element;
	while (true)
	{
		char c = data.get();
		if (c == '>') break;
		if (c == -1) break;
		element.append(1, c);
	}
	return element;
}

std::string TextToken(std::ifstream& data) {
	std::string text;
	while (true)
	{
		char c = data.get();
		if (c == '<') break;
		if (c == -1) break;
		if (c == '\n') continue;
		if (c == '\t') continue;
		else
		{
			text.append(1, c);
		}
		
	}
	return text;
}


Element ParseXML::Tokenizer(std::ifstream& data)
{
	Element element;
	std::string startElement;
	std::string nextElement;
	std::string text;


	char current = data.get();
	if (current == -1) {
		std::cout << __FUNCTION__": End of file reached before all elements are closed. Cant read XML file." << std::endl;
		return element;
	}
	if (current == '<') {
		startElement = ElementToken(data);
		text = TextToken(data);
		nextElement = ElementToken(data);
	}
	else {
		std::cout << __FUNCTION__": No text data should exist before root of the file. Cant read XML file." << std::endl;
		return element;
	}
	

	//If next element is closing starting element, then it has no children.
	//Element name is same as startElement and text of element is text between startElement and endElement 
	if (nextElement == "/" + startElement) {
		
		//Ignores things after closing element and moves position in file to next element
		int b = static_cast<int>(data.tellg());
		data.seekg(b - 1);
		TextToken(data); 
		b = static_cast<int>(data.tellg());
		data.seekg(b - 1);

		element.name = startElement;
		element.textContent = text;
		return element;
	}

	//If nextElement does not close startElement, then nextElement is child of startElement.
	if(nextElement != "/" + startElement) {
		
		//Returns file pointer to before nextElement
		int reset_pos = static_cast<int>(data.tellg());
		data.seekg(reset_pos - nextElement.size() - 2);

		while (true) {

			//Reads next element, expecting to find closing element.
			
			auto endingElement = ElementToken(data);
			int pos = static_cast<int>(data.tellg());
			data.seekg(pos - endingElement.size() - 1);
			pos = static_cast<int>(data.tellg());
			int p2 = static_cast<int>(data.tellg());
			
			//If closing element is there
			if (endingElement == ("</" + startElement)) {
				element.name = startElement;
				element.textContent = text;

				endingElement = ElementToken(data);
				TextToken(data);
				
				int pos = static_cast<int>(data.tellg());
				data.seekg(pos - 1);

				break;
			}

			//If does next element is not closing element then they are children
			//and they are added to children's list of the element
			else 
			{
				Element e = Tokenizer(data);
				if (e.name.empty()) break;
				element.children.push_back(e);
			}
		}
		return element;
	}
	
	else
	{
		return element;
	}
	
}

ParseXML::ParseXML(const std::string& file)
{
	std::ifstream data = std::ifstream(file);
	elements = Tokenizer(data);
	

	data.close();
}

Element ParseXML::getElement(const std::string& name)
{
	for (int i = 0; i < elements.children.size(); i++) 
	{
		Element childElement = elements.children[i];
		if (childElement.name == name) {
			return childElement;
		}
	}
	std::cout << __FUNCTION__": cant get an element with name " << name << std::endl;
	return Element();
}

SmoothieMath::Vector3 Element::getVector3()
{
	std::istringstream data(textContent);
	
	Vector3 result;
	
	float x, y, z;
	
	data >> x >> y >> z;

	result.setX(x);
	result.setY(y);
	result.setZ(z);

	return result;
}


SmoothieMath::Vector4 Element::getVector4()
{
	std::istringstream data(textContent);


	float x, y, z, w;

	data >> x >> y >> z >> w;
	Vector4 result(x, y, z, w);
	
	return result;
}

bool Element::getBool()
{
	if (textContent == "true") return true;
	return false;
}

int Element::getInt()
{
	std::istringstream data(textContent);
	int number;
	data >> number;
	return number;
}

float Element::getFloat()
{
	std::istringstream data(textContent);
	float number;
	data >> number;
	return number;
}

Element* Element::getChild(const std::string& name)
{
	for (int i = 0; i < children.size(); i++) 
	{
		std::string elementName = children[i].name;
		if (elementName == name) 
		{
			return &children[i];
		}
	}
	return nullptr;
}

bool Element::hasChild(const std::string& name) const
{
	for (int i = 0; i < children.size(); i++)
	{
		std::string elementName = children[i].name;
		if (elementName == name)
		{
			return true;
		}
	}
	return false;
}
