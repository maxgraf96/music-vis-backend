//
// Created by Max on 05/08/2020.
//

#ifndef MUSIC_VIS_BACKEND_METAPARAMETERFLOAT_H
#define MUSIC_VIS_BACKEND_METAPARAMETERFLOAT_H

#include <juce_audio_processors/juce_audio_processors.h>

using namespace juce;

class MetaParameterFloat : public AudioParameterFloat{
public:
    MetaParameterFloat(String parameterID, String parameterName, float minValue, float maxValue, float defaultValue);
    bool isMetaParameter() const override;
};


#endif //MUSIC_VIS_BACKEND_METAPARAMETERFLOAT_H
