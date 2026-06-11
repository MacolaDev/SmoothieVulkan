#include "XML.h"
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace SmoothieMath;
using namespace Smoothie::XML;

enum DATA_TYPE
{
	DATA_TYPE_ELEMENT,
	DATA_TYPE_TEXT
};

struct DataDescription
{
	DATA_TYPE type;
	std::string data;
};

enum TOKEN_VALUE
{
	TOKEN_VALUE_UNDEFINED = 0,
	TOKEN_VALUE_ELEMENT_STANDARD,
	TOKEN_VALUE_ELEMENT_QUESTIONMARK,
	TOKEN_VALUE_ELEMENT_EXCLAMATIONMARK,
	TOKEN_VALUE_TEXT
};

struct Token
{
	TOKEN_VALUE type = TOKEN_VALUE_UNDEFINED;
	std::string value;
};

struct TreeBase
{
	std::string start = "";
	std::string text = "";
};

static inline bool isCharSpaceChar(char c)
{
	return (c == 0x20) || (c == 0x9) || (c == 0xD) || (c == 0xA);
}

static inline bool isCharDigit(char c)
{
	if (std::isdigit(c)) return true;
	return false;
}

static inline bool isCharLetter(char c)
{
	if (std::isalpha(c)) return true;
	return false;
}

static int getElementToken(const std::string& data, size_t& position, size_t& line, Token& token, XMLError& error)
{
	token.value.clear();
	while (position < data.size())
	{
		char c = data[position];
		if (c == '>')
		{
			position--;
			break;
		}

		if (c == '\n') line++;

		token.value.push_back(c);
		position++;
	}

	if (token.value.empty())
	{
		token.type = TOKEN_VALUE_UNDEFINED;
		error.line = line;
		error.message = "Empty element not allowed!";
		return -1;
	}

	if (token.value[0] == '!')
	{
		token.type = TOKEN_VALUE_ELEMENT_EXCLAMATIONMARK;
	}
	else if (token.value[0] == '?')
	{

		token.type = TOKEN_VALUE_ELEMENT_QUESTIONMARK;
	}
	else
	{
		token.type = TOKEN_VALUE_ELEMENT_STANDARD;
	}

	return 0;
}

static int getTextToken(const std::string& data, size_t& position, size_t& line, Token& token, XMLError& error)
{
	token.value.clear();
	token.type = TOKEN_VALUE_TEXT;

	bool ASCIISymbolEncauntered = false;
	while (position < data.size())
	{
		char c = data[position];
		if (c == '<')
		{
			//token.value.push_back(0);
			position--;
			break;
		}
		ASCIISymbolEncauntered |= isCharDigit(c);
		ASCIISymbolEncauntered |= isCharLetter(c);

		if (c == '\n') line++;

		if (isCharSpaceChar(c) && (ASCIISymbolEncauntered == false))
		{
			position++;
			continue;
		}

		token.value.push_back(c);
		position++;
	}


	if (token.value.size() == 1)
	{
		token.value.clear();
		return 0;
	}
	return 0;
}

static int getElementFromToken(const std::string& tokenData, Element& element, bool& isClosingElement, bool& noChildElement, XMLError& error)
{
	size_t position = 0;

	if (tokenData.size() < 2) return -1;

	if (tokenData[0] == '/') isClosingElement = true;

	//get element name
	element.name.clear();
	for (size_t i = position; i < tokenData.size(); i++)
	{
		if (tokenData[i] == ' ') break;
		element.name.push_back(tokenData[i]);
		position++;
	}

	if (tokenData[tokenData.size() - 1] == '/') noChildElement = true;

	if (position == tokenData.size()) return 0;
	if (isClosingElement == true) return 0; //Closing element does not have any attributes



	std::string attributes;
	attributes.resize(tokenData.size() - position);
	//std::strcpy(&attributes[0], &tokenData[position]);

	return 0;
}

static int buildTree(const std::vector<TreeBase>& treeData, size_t& position, Element& rootElement, bool& isClosingElement, XMLError& error)
{
	if (position >= treeData.size())
	{
		error.line = 0;
		error.message = "Failed to build xml tree!";
		return -1;
	}

	Element element;
	bool noChildElement = false;
	if (getElementFromToken(treeData[position].start, element, isClosingElement, noChildElement, error) != 0) return -1;
	element.textContent = treeData[position].text;
	position++;

	if (isClosingElement || noChildElement)
	{
		rootElement = element;
		return 0;
	}

	Element elementNext;
	while (elementNext.name != ("/" + element.name))
	{
		bool element_end = false;
		if (buildTree(treeData, position, elementNext, element_end, error) != 0) return -1;
		if (!element_end) element.children.push_back(elementNext);
	}


	rootElement = element;
	return 0;
}

static int parse(const std::string& data, XMLError& error, Element& baseElement)
{

	//Fast check for open and closing elements
	unsigned int elementEnd = 0;
	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i] == '<') elementEnd++; //Element is open
		if (data[i] == '>') elementEnd--; //Element is closed
	}

	if (elementEnd != 0)
	{
		error.message = "Every \"<\" must be closed with \">\"!";
		return -1;
	}

	if (data[0] != '<')
	{
		error.message = "Invalid token at the start of the file!";
		return -1;
	}


	size_t position = 0;
	size_t line = 0;
	std::vector<Token> tokens;
	while (position < data.size())
	{
		char c = data[position];
		Token token;

		//Element start
		if (c == '<')
		{
			position++;
			if (getElementToken(data, position, line, token, error) != 0) return -1;
			if (token.type == TOKEN_VALUE_ELEMENT_STANDARD) tokens.push_back(token);
		}

		//Text start
		else if (c == '>')
		{
			position++;
			if (getTextToken(data, position, line, token, error) != 0) return -1;
			if (!token.value.empty()) tokens.push_back(token);
		}


		if (c == '\n') line++;
		position++;
	}

	if (tokens.empty()) return 0;


	std::vector<TreeBase> treeBasis;
	position = 0;
	while (position < tokens.size() - 1)
	{
		TreeBase base;
		const auto& token0 = tokens[position + 0];
		const auto& token1 = tokens[position + 1];

		if (token0.type == TOKEN_VALUE_TEXT)
		{
			position++;
			continue;
		}
		else
		{
			base.start = token0.value;
		}

		if (token1.type == TOKEN_VALUE_TEXT)
		{
			base.text = token1.value;
		}
		else
		{
			base.text = "";
		}
		treeBasis.push_back(base);
		position++;
	}


	treeBasis.push_back({ tokens.back().value, "" });
	position = 0;
	bool isClosingElement = false;
	if (buildTree(treeBasis, position, baseElement, isClosingElement, error) != 0) return -1;

	return 0;
}

int Smoothie::XML::built_tree_from_string(const std::string& data, Element& element, XMLError& error)
{
	if (parse(data, error, element)) return -1;
	return 0;
}

int Smoothie::XML::build_tree_from_xml_file(const std::string& filepath, Element& element, XMLError& error)
{

	if (!std::filesystem::exists(filepath))
	{
		error.message = "File: " + filepath + "Does not exist!";
		return -1;
	}

	std::ifstream file(filepath, std::ifstream::binary);
	if (!file)
	{
		error.message = "Can't open file: " + filepath;
		return -1;
	}
	auto fileSize = std::filesystem::file_size(filepath);

	std::string dataFromFile(fileSize, 0);
	file.read(dataFromFile.data(), fileSize);
	file.close();

	if (parse(dataFromFile, error, element)) return -1;

	dataFromFile.clear();
	return 0;
}

static int buidString(const Element& element, std::string& string)
{

	if (element.children.empty() && element.textContent.empty())
	{
		string.push_back('<');
		string += element.name;
		for (const auto& attribute : element.attributes)
		{
			std::string attribute_str = " ";

			attribute_str += attribute.name + "=\"" + attribute.value + "\" ";
			string += attribute_str;
		}

		string.push_back('/');
		string.push_back('>');


		return 0;
	}

	string.push_back('<');
	string += element.name;

	for (const auto& attribute : element.attributes)
	{
		std::string attribute_str = " ";

		attribute_str += attribute.name + "=\"" + attribute.value + "\" ";
		string += attribute_str;
	}

	string.push_back('>');

	string += element.textContent;


	for (const auto& child : element.children)
	{
		if (buidString(child, string) != 0) return -1;
	}

	string.push_back('<');
	string.push_back('/');
	string += element.name;
	string.push_back('>');

	return 0;
}

int Smoothie::XML::build_string_from_tree(const Element& element, std::string& string, XMLError& error)
{
	string.clear();
	return buidString(element, string);
}

int Smoothie::XML::build_xml_file_from_tree(const Element& element, const std::string& filepath, XMLError& error)
{
	std::string data = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";

	if (buidString(element, data) != 0)
	{
		error.message = "Failed to build string from Element tree!";
		return -1;
	}

	std::ofstream file(filepath, std::ofstream::binary);
	if (!file)
	{
		error.message = "Can't open file: " + filepath + " for writhing!";
		return -1;
	}

	file.write(data.c_str(), data.size());
	file.close();
	return 0;
}

SmoothieMath::Vector2 Smoothie::XML::Element::getVector2() const
{
	std::istringstream data(textContent);
	Vector2 result;
	data >> result.x >> result.y;
	return result;
}

SmoothieMath::Vector3 Element::getVector3() const
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

SmoothieMath::Vector4 Element::getVector4() const
{
	std::istringstream data(textContent);


	float x, y, z, w;

	data >> x >> y >> z >> w;
	Vector4 result(x, y, z, w);
	
	return result;
}

bool Element::getBool() const
{
	if (textContent == "true") return true;
	return false;
}

int Element::getInt() const
{
	std::istringstream data(textContent);
	int number;
	data >> number;
	return number;
}

float Element::getFloat() const
{
	std::istringstream data(textContent);
	float number;
	data >> number;
	return number;
}

Element Element::getChild(const std::string& name) const
{
	for (size_t i = 0; i < children.size(); i++) 
	{
		const std::string& elementName = children[i].name;
		if (elementName == name) 
		{
			return children[i];
		}
	}
	return Element();
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
