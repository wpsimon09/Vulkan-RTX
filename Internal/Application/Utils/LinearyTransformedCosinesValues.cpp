//
// Created by wpsimon09 on 08/02/25.
//
#include "LinearyTransformedCosinesValues.hpp"

void MathUtils::InitLTC()
{
    LTC_ImageData = {MathUtils::LTC, 64, 64, 4, MathUtils::LTC_String};
    LTCInverse_ImageData = {MathUtils::LTC_Inverse, 64, 64, 4, MathUtils::LTC_InverseString};
}
