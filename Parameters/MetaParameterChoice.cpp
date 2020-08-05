//
// Created by Max on 05/08/2020.
//

#include "MetaParameterChoice.h"

MetaParameterChoice::MetaParameterChoice(const String& parameterID, const String& parameterName, const StringArray& choices, int defaultItemIndex)
    : AudioParameterChoice(parameterID, parameterName, choices, defaultItemIndex){
}

bool MetaParameterChoice::isMetaParameter() const {
    return true;
}

