//
// Created by Max on 07/06/2020.
//

#ifndef MUSIC_VIS_BACKEND_CONSTANTS_H
#define MUSIC_VIS_BACKEND_CONSTANTS_H

#include <cmath>
#include <juce_audio_processors/juce_audio_processors.h>

// sqrt(2)/2 for standard filter quality value
const double SQRT_2_OVER_2 = sqrt(2.0) / 2.0;

// Algorithms supported by feature slots
static juce::StringArray featureSlotAlgorithmOptions = { "-", "Loudness", "Spectral Centroid" };

// Number of feature slots per band
const int NUMBER_OF_SLOTS = 2;

// Number of automatables
const int NUMBER_OF_AUTOMATABLES = 5;

#endif //MUSIC_VIS_BACKEND_CONSTANTS_H
