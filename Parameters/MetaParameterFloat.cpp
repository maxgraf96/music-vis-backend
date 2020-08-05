//
// Created by Max on 05/08/2020.
//

#include "MetaParameterFloat.h"

MetaParameterFloat::MetaParameterFloat(String parameterID, String parameterName, float minValue, float maxValue, float defaultValue)
        : AudioParameterFloat(parameterID, parameterName, minValue, maxValue, defaultValue) {
}

bool MetaParameterFloat::isMetaParameter() const {
    return true;
}

