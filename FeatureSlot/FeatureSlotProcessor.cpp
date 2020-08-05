//
// Created by Max on 14/06/2020.
//

#include "FeatureSlotProcessor.h"

FeatureSlotProcessor::FeatureSlotProcessor(mapper::Device& libmapperDev, foleys::MagicProcessorState& ms, Band b, vector<Real>& inputBuffer, int slotNo):
        libmapperDevice(libmapperDev), magicState(ms), band(b), inputAudioBuffer(inputBuffer), slotNumber(slotNo) {
    // Get connected property from valueTreeState
    std::string algoProp = band == LOW ? "low" : band == MID ? "mid" : "high";
    algoProp.append("Slot").append(to_string(slotNo));

    paramID = algoProp;
    magicState.getValueTreeState().addParameterListener(algoProp, this);

    // Initialise algorithm if selected
    int paramVal = magicState.getValueTreeState().getParameterAsValue(algoProp).getValue();
    String algoName = featureSlotAlgorithmOptions.getReference(paramVal);
    initialiseAlgorithm(algoName);

    String val = algoProp.append("Value");
    outputValue.referTo(magicState.getPropertyAsValue(val));

    sensor = make_unique<mapper::Signal>(libmapperDevice.add_output_signal(algoProp.insert(0, "sub_"), 1, 'f', 0, 0, 0));
    sensor->set_rate(30);

    // Start timer for GUI updates
    stopTimer();
    startTimer(30);
}

FeatureSlotProcessor::~FeatureSlotProcessor() {
    magicState.getValueTreeState().removeParameterListener(paramID, this);
}

void FeatureSlotProcessor::compute() {
    if(algorithm != nullptr && !isAlgorithmChanging.load()){
        algorithm->compute();
        // Update output value for label
        currentValue = outputScalar;
    }
}

FeatureSlotProcessor::Band FeatureSlotProcessor::getBand() {
    return band;
}

void FeatureSlotProcessor::setBand(Band band){
    this->band = band;
}

void FeatureSlotProcessor::initialiseAlgorithm(String algoStr) {
    // Big switcharoo for algorithm initialisation
    if(algoStr == "-"){
        algorithm.reset(nullptr);
    }
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

    // Add new signal
    std::string signalName = band == LOW ? "Low" : band == MID ? "Mid" : "High";
    signalName.append("_Slot_").append(to_string(slotNumber)); //.append(":").append(algoStr.toStdString());
    // Remove any whitespaces from string, as libmapper doesn't support spaces in signal names :(((
    signalName.erase (std::remove (signalName.begin(), signalName.end(), ' '), signalName.end());
}

Value &FeatureSlotProcessor::getOutputValue() {
    return outputValue;
}

void FeatureSlotProcessor::parameterChanged(const String &parameterID, float newValue) {
    if(parameterID == paramID){
        isAlgorithmChanging.store(true);
        currentValue = 0;

        if(algorithm != nullptr){
            algorithm->reset();
            algorithm.reset(nullptr);
        }
        // Convert choice index to int
        int idx = static_cast<int>(newValue) + 1;
        // If no item is selected don't initialise an algorithm
        if(idx == 0){
            return;
        }

        // Get name of algorithm
        String algoName = featureSlotAlgorithmOptions.getReference(idx - 1);
        // Update if a new algorithm was selected
        if(currentAlgoString != algoName){
            currentAlgoString = algoName;
            initialiseAlgorithm(algoName);
        }

        isAlgorithmChanging.store(false);
    }
}

void FeatureSlotProcessor::timerCallback() {
    if(algorithm != nullptr && !isAlgorithmChanging.load()){
    outputValue.setValue(currentValue);

    libmapperDevice.poll();
    sensor->update(currentValue);
    }
}
