#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Math/SmoothieMath.h"

namespace Smoothie 
{
    namespace XML
    {

        struct Attribute
        {
            std::string name;
            std::string value;
        };

        struct Element
        {
            std::string name;
            std::string textContent;
            std::vector<Element> children;
            std::vector<Attribute> attributes;

            //Converts textContent to Vector3 value
            SmoothieMath::Vector2 getVector2() const;

            //Converts textContent to Vector3 value
            SmoothieMath::Vector3 getVector3() const;

            //Converts textContent to Vector4 value
            SmoothieMath::Vector4 getVector4() const;

            //Converts textContent to bool value
            bool getBool() const;

            //Converts textContent to int value
            int getInt() const;

            //Converts textContent to float value
            float getFloat() const;

            //If element with a name "name" exists, it returns a copy of it.
            //Othervise it returns a new element
            Element getChild(const std::string& name) const;

            //Returns true if child with name "name" exists.
            //Otherwise it returns false.
            bool hasChild(const std::string& name) const;

            Element(const std::string& elementName, const std::string& elementText = "") :name(elementName), textContent(elementText) {};
            
            Element() = default;
        };

        struct XMLError
        {
            size_t line = 0;
            std::string message = "";
        };

        int built_tree_from_string(const std::string& data, Element& element, XMLError& error);
        int build_tree_from_xml_file(const std::string& filepath, Element& element, XMLError& error);

        int build_string_from_tree(const Element& element, std::string& string, XMLError& error);
        int build_xml_file_from_tree(const Element& element, const std::string& filepath, XMLError& error);

    }
}

