//
// Created by Max on 14/06/2020.
//

#include "FeatureSlotProcessor.h"

FeatureSlotProcessor::FeatureSlotProcessor(mapper::Device& libmapperDev, foleys::MagicProcessorState& ms, Band b, vector<Real>& inputBuffer, int slotNo):
        libmapperDevice(libmapperDev), magicState(ms), band(b), inputAudioBuffer(inputBuffer), slotNumber(slotNo) {
    // Get connected property from state management
    std::string algoProp = band == LOW ? "low" : band == MID ? "mid" : "high";
    algoProp.append("Slot").append(to_string(slotNo));

    // Connect parameter listener with state management
    paramID = algoProp;
    magicState.getValueTreeState().addParameterListener(algoProp, this);

    // Initialise algorithm if one is selected
    int paramVal = magicState.getValueTreeState().getParameterAsValue(algoProp).getValue();
    String algoName = featureSlotAlgorithmOptions.getReference(paramVal);
    initialiseAlgorithm(algoName);

    // Connect to value in state management
    String val = algoProp.append("Value");
    outputValue.referTo(magicState.getPropertyAsValue(val));

    // Create libmapper signal for this FeatureSlot
    sensor = make_unique<mapper::Signal>(libmapperDevice.add_output_signal(algoProp.insert(0, "sub_"), 1, 'f', 0, 0, 0));
    // Limit transmission rate to 30 times per second
    // Note: This has no impact on the frame rate in the frontend
    sensor->set_rate(30);

    // Start timer for GUI updates
    stopTimer();
    startTimer(30);
}

FeatureSlotProcessor::~FeatureSlotProcessor() {
    magicState.getValueTreeState().removeParameterListener(paramID, this);
}

void FeatureSlotProcessor::compute() {
    // Only compute if there is an algorithm selected and the algorithm is not currently changing.
    if(algorithm != nullptr && !isAlgorithmChanging.load()){
        // Compute output value
        algorithm->compute();

        // Update output value for label
        currentValue = outputScalar;
    }
}

void FeatureSlotProcessor::initialiseAlgorithm(String algoStr) {
    // Algorithm initialisation
    // If there is no algorithm selected, reset the algorithm field to nullptr
    if(algoStr == "-"){
        algorithm.reset(nullptr);
    }
    // Otherwise initialise with respective algorithm
    if(algoStr == "Loudness"){
        algorithm.reset(factory.create("Loudness"));
        algorithm->input("signal").set(inputAudioBuffer);
        algorithm->output("loudness").set(outputScalar);
    }
    if(algoStr == "Spectral Centroid"){
        algorithm.reset(factory.create("SpectralCentroidTime", "sampleRate", (double) magicState.getPropertyAsValue("sampleRate").getValue()));
        algorithm->input("array").set(inputAudioBuffer);
        algorithm->output("centroid").set(outputScalar);
    }
}

Value &FeatureSlotProcessor::getOutputValue() {
    return outputValue;
}

void FeatureSlotProcessor::parameterChanged(const String &parameterID, float newValue) {
    if(parameterID == paramID){
        // Block further computation calls while algorithm is changing
        isAlgorithmChanging.store(true);
        // Reset output value
        currentValue = 0;

        // Reset algorithm field if no algorithm selected
        if(algorithm != nullptr){
            algorithm->reset();
            algorithm.reset(nullptr);
        }

        // Convert index from combobox items to int
        int idx = static_cast<int>(newValue) + 1;
        // If no item is selected don't initialise an algorithm
        if(idx == 0){
            isAlgorithmChanging.store(false);
            return;
        }

        // Get name of algorithm
        String algoName = featureSlotAlgorithmOptions.getReference(idx - 1);
        // Update if a new algorithm was selected
        if(currentAlgoString != algoName){
            currentAlgoString = algoName;
            initialiseAlgorithm(algoName);
        }

        // Reset flag and thereby re-enable computation
        isAlgorithmChanging.store(false);
    }
}

void FeatureSlotProcessor::timerCallback() {
    if(algorithm != nullptr && !isAlgorithmChanging.load()){
        // Update the output value
        outputValue.setValue(currentValue);

        // Poll libmapper device
        libmapperDevice.poll();
        // Update signal value
        sensor->update(currentValue);
    }
}
