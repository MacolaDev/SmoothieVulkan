#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Math/SmoothieMath.h"

struct Element 
{
    std::string name;
    std::string textContent;
    std::vector<Element> children;

    //Converts textContent to Vector3 value
    SmoothieMath::Vector3 getVector3();

    //Converts textContent to Vector4 value
    SmoothieMath::Vector4 getVector4();

    //Converts textContent to bool value
    bool getBool();

    //Converts textContent to int value
    int getInt();

    //Converts textContent to float value
    float getFloat();

    //Returns a pointer to a child of selected element with name "name".
    //Returns nullptr if child does not exist.
    Element* getChild(const std::string& name);

    //Returns true if child with name "name" exists.
    //Otherwise it returns false.
    bool hasChild(const std::string& name) const;
};

class ParseXML
{
    Element Tokenizer(std::ifstream& data);
    Element elements;
public:
    ParseXML(const std::string& file);
    ParseXML() {
        elements.name = "";
        elements.textContent = "";
    }
    //Goes throut root of xml file, trying to find element with elementName.
    //If element is found, it returns. If not, empty element is returned and error is printed.
    Element getElement(const std::string& elementName);

};
