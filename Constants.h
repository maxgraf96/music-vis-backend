//
// Created by Max on 07/06/2020.
//

#ifndef MUSIC_VIS_BACKEND_CONSTANTS_H
#define MUSIC_VIS_BACKEND_CONSTANTS_H

#include <cmath>

const double SQRT_2_OVER_2 = sqrt(2) / 2;

// Algorithms supported by feature slots
static juce::StringArray featureSlotAlgorithmOptions = {"Loudness", "Spectral Centroid"};

// Number of feature slots per band
const int NUMBER_OF_SLOTS = 2;

#endif //MUSIC_VIS_BACKEND_CONSTANTS_H
