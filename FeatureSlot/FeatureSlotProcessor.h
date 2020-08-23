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

/**
 * Backend for the FeatureSlot component.
 * Feature slots are used in the sub-bands. They can be configured to compute specific algorithms.
 * The underlying algorithm they compute can be changed at runtime.
 */
class FeatureSlotProcessor : private AudioProcessorValueTreeState::Listener, Timer {
public:

    /**
     * Enum for indicating the sub-band the FeatureSlot is assigned to
     */
    enum Band {
        LOW,
        MID,
        HIGH
    };

    FeatureSlotProcessor(mapper::Device&, foleys::MagicProcessorState&, Band, vector<Real>&, int);
    ~FeatureSlotProcessor();

    /**
     * Initialise one of the available Essentia algorithms
     * @param algoStr The Essentia descriptor of the algorithm, e.g. "SpectralCentroidTime" for the spectral centroid
     */
    void initialiseAlgorithm(String algoStr);

    /**
     * Getter for the current output value of the currently selected algorithm
     * @return
     */
    Value& getOutputValue();

    /**
     * Performs the computation of the selected algorithm using the currently available input data (see field inputAudioBuffer)
     */
    void compute();

    /**
     * Timer callback that performs the computation if an algorithm is selected
     */
    void timerCallback() override;

private:
    // State management
    foleys::MagicProcessorState& magicState;
    String paramID = "";

    // Essentia identifier of the current algorithm
    String currentAlgoString = "";

    // The input buffer for this slot
    const vector<Real>& inputAudioBuffer;

    // This field will contain the output if the output is a scalar value
    Real outputScalar = -1.0f;
    // This field will contain the output value
    Value outputValue;

    // Important: Placeholder for the most recent computation result, which is updated by a timed function in order
    // to avoid GUI update calls from the audio thread
    float currentValue = 0.0f;

    // Factory for creating the algorithm
    standard::AlgorithmFactory& factory = standard::AlgorithmFactory::instance();

    // The Essentia algorithm object for this slot
    unique_ptr<Algorithm> algorithm;

    // Flag indicating whether the algorithm is currently being changed
    // Important to handle calls to the compute() function while the algorithm is in the process of changing
    atomic<bool> isAlgorithmChanging = ATOMIC_VAR_INIT(false);

    // Reference to the main libmapper device
    mapper::Device& libmapperDevice;
    // Libmapper signal for this FeatureSlot
    unique_ptr<mapper::Signal> sensor;

    // Indicator of the sub-band of the FeatureSlot
    Band band = LOW;
    // Indicator of the number of the slot in its respective sub-band
    int slotNumber = -1;

    /**
     * Callback when algorithm is changed via the GUI
     * @param parameterID
     * @param newValue The index of the of the new algorithm's Essentia identifier string
     * (see field featureSlotAlgorithmOptions in Constants.h)
     */
    void parameterChanged(const String& parameterID, float newValue) override;

};


#endif //MUSIC_VIS_BACKEND_FEATURESLOTPROCESSOR_H
