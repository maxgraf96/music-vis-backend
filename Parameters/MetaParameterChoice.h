//
// Created by Max on 05/08/2020.
//

#ifndef MUSIC_VIS_BACKEND_METAPARAMETERCHOICE_H
#define MUSIC_VIS_BACKEND_METAPARAMETERCHOICE_H

#include <juce_audio_processors/juce_audio_processors.h>

using namespace juce;

class MetaParameterChoice : public AudioParameterChoice {
public:
    MetaParameterChoice(const String& parameterID, const String& parameterName, const StringArray& choices, int defaultItemIndex);
    bool isMetaParameter() const override;
};


#endif //MUSIC_VIS_BACKEND_METAPARAMETERCHOICE_H
