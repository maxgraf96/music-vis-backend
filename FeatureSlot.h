//
// Created by Max on 14/06/2020.
//

#ifndef MUSIC_VIS_BACKEND_FEATURESLOT_H
#define MUSIC_VIS_BACKEND_FEATURESLOT_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <essentia/algorithmfactory.h>
#include <mapper/mapper_cpp.h>
#include "foleys_gui_magic/foleys_gui_magic.h"
#include "Constants.h"

using namespace std;
using namespace juce;
using namespace essentia;
using namespace essentia::standard;

class FeatureSlot : public Component, Value::Listener {
public:

    enum Band {
        LOW,
        MID,
        HIGH
    };

    FeatureSlot(mapper::Device& libmapperDevice, foleys::MagicProcessorState& magicState);
    ~FeatureSlot();

    void initialiseAlgorithm(String algoStr);
    void attachToParameter(const String& value, AudioProcessorValueTreeState& vts);

    void compute();

    void paint (Graphics&) override;
    void resized() override;

    void setBand(Band band);
    Band getBand();

    void valueChanged (Value &value) override;
    void setInputAudioBuffer(shared_ptr<vector<Real>> audioBuffer);
    void setSlotNumber(int number);

private:
    // State management
    foleys::MagicProcessorState& magicState;

    // The input buffer for this slot
    shared_ptr<vector<Real>> inputAudioBuffer;
    // Will contain the output if the output is a scalar value
    Real outputScalar = -1.0f;
    Value outputValue;

    // Factory for creating the algorithm
    standard::AlgorithmFactory& factory = standard::AlgorithmFactory::instance();

    // The algorithm for this slot
    unique_ptr<Algorithm> algorithm;

    // Reference to the main libmapper device
    mapper::Device& libmapperDevice;
    unique_ptr<mapper::Signal> sensor;

    Band band = LOW;

    int slotNumber = -1;

    // Display stuff
    unique_ptr<foleys::AttachableComboBox> algorithmComboBox;
    unique_ptr<Label> valueLabel;
};


#endif //MUSIC_VIS_BACKEND_FEATURESLOT_H
