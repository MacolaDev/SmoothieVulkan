//
// Created by macola on 7/3/26.
//

#include "Helper.h"

using namespace Smoothie::AssetPipeline::Geometry_Helpers;
using namespace Smoothie;

int Smoothie::AssetPipeline::Geometry_Helpers::get_type_size(Geometry_Data_Type::Base_Type type)
{
    switch (type)
    {
        case Geometry_Data_Type::Base_Type::Int_8:
        case Geometry_Data_Type::Base_Type::UInt_8:
        {
            return 1;
        }break;

        case Geometry_Data_Type::Base_Type::Int_16:
        case Geometry_Data_Type::Base_Type::UInt_16:
        {
            return 2;
        }break;

        case Geometry_Data_Type::Base_Type::Int_32:
        case Geometry_Data_Type::Base_Type::UInt_32:
        case Geometry_Data_Type::Float_32:
        {
            return 4;
        }break;

        case Geometry_Data_Type::Base_Type::Int_64:
        case Geometry_Data_Type::Base_Type::UInt_64:
        {
            return 8;
        }break;

        default: {return 0;}break;
    }
}

