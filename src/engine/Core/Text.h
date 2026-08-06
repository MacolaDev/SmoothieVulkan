#pragma once
#include "Common.h"

namespace Smoothie
{
    enum Text_Node_Type
    {
        Text_Node_Type_Base = 0,

        Text_Node_Type_String,
        Text_Node_Type_Int32,
        Text_Node_Type_Float32,

        Text_Node_Type_Array,

        Text_Node_Type_Array_String,
        Text_Node_Type_Array_Int32,
        Text_Node_Type_Array_Float32,

        Text_Node_Type_Map
    };

    struct Text_Node_Base
    {
        const Text_Node_Type type;

        explicit Text_Node_Base(Text_Node_Type type = Text_Node_Type_Base) : type(Text_Node_Type_Base) {}
        virtual ~Text_Node_Base() = default;
    };

    template<typename T, Text_Node_Type _type>
    struct Text_Node : public Text_Node_Base
    {
        T data;
        explicit Text_Node() : Text_Node_Base(_type) {}
    };

    using Text_Node_String = Text_Node<std::string, Text_Node_Type_String>;
    using Text_Node_Int32 = Text_Node<int, Text_Node_Type_Int32>;
    using Text_Node_Float32 = Text_Node<float, Text_Node_Type_Float32>;
    using Text_Node_Array = Text_Node<std::vector<std::unique_ptr<Text_Node_Base>>, Text_Node_Type_Array>;
    using Text_Node_Array_String = Text_Node<std::vector<std::string>, Text_Node_Type_Array_String>;
    using Text_Node_Array_Int32 = Text_Node<std::vector<int>, Text_Node_Type_Int32>;
    using Text_Node_Array_Float32 = Text_Node<std::vector<float>, Text_Node_Type_Float32>;
    using Text_Map = std::unordered_map<std::string, std::unique_ptr<Text_Node_Base>>;
    using Text_Node_Map = Text_Node<Text_Map, Text_Node_Type_Map>;

    class TextFile
    {
    public:

        int load(const std::string& filepath);

        Text_Map::iterator begin() { return m_RootNode.begin(); }
        Text_Map::iterator end() { return m_RootNode.end(); }

        static constexpr const char* getMagic() {return s_Magic;}
    private:

        static constexpr char s_Magic[4] = "216";
        Text_Map m_RootNode;

    };
}

