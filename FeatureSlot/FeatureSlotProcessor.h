//
// Created by Max on 14/06/2020.
//

#ifndef MUSIC_VIS_BACKEND_FEATURESLOTPROCESSOR_H
#define MUSIC_VIS_BACKEND_FEATURESLOTPROCESSOR_H

#include <juce_audio_processors/juce_audio_processors.h>
#include "../external_libraries/essentia/include/algorithmfactory.h"
#include <mapper/mapper_cpp.h>
#include "../foleys_gui_magic/foleys_gui_magic.h"
#include "../Constants.h"

using namespace std;
using namespace juce;
using namespace essentia;
using namespace essentia::standard;

class FeatureSlotProcessor : private AudioProcessorValueTreeState::Listener {
public:

    enum Band {
        LOW,
        MID,
        HIGH
    };

    FeatureSlotProcessor(mapper::Device&, foleys::MagicProcessorState&, Band, vector<Real>&, int);
    ~FeatureSlotProcessor();

    void initialiseAlgorithm(String algoStr);

    Value& getOutputValue();

    void compute();

    void setBand(Band band);
    Band getBand();

private:
    // State management
    foleys::MagicProcessorState& magicState;
    String paramID = "";

    std::mutex mutex;

    String currentAlgoString = "";

    // The input buffer for this slot
    const vector<Real>& inputAudioBuffer;
    // Will contain the output if the output is a scalar value
    Real outputScalar = -1.0f;
    Value outputValue;

    // Factory for creating the algorithm
    standard::AlgorithmFactory& factory = standard::AlgorithmFactory::instance();

    // The algorithm for this slot
    unique_ptr<Algorithm> algorithm;
    atomic<bool> isAlgorithmChanging = ATOMIC_VAR_INIT(false);

    // Reference to the main libmapper device
    mapper::Device& libmapperDevice;
    unique_ptr<mapper::Signal> sensor;

    Band band = LOW;

    int slotNumber = -1;

    void parameterChanged(const String& parameterID, float newValue) override;

};


#endif //MUSIC_VIS_BACKEND_FEATURESLOTPROCESSOR_H
