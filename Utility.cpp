//
// Created by Max on 24/05/2020.
//

#include "Utility.h"

float mapFloat(float in, float inStart, float inEnd, float outStart, float outEnd){
    auto slope = 1.0f * (outEnd - outStart) / (inEnd - inStart);
    return outStart + slope * (in - inStart);
}

