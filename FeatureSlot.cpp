//
// Created by Max on 14/06/2020.
//

#include "FeatureSlot.h"

#include <utility>

FeatureSlot::FeatureSlot(mapper::Device& libmapperDev, foleys::MagicProcessorState& mState):
        libmapperDevice(libmapperDev), magicState(mState){

    algorithmComboBox = make_unique<foleys::AttachableComboBox>();
    valueLabel = make_unique<Label>("displayValue", "");
    valueLabel->getTextValue().referTo(outputValue);

    // Add to parent
    addAndMakeVisible(*algorithmComboBox);
    addAndMakeVisible(*valueLabel);

    algorithmComboBox->getSelectedIdAsValue().addListener(this);

    setSize(200, 100);
}

FeatureSlot::~FeatureSlot() {
}

void FeatureSlot::paint(Graphics &) {

}

void FeatureSlot::resized() {
    // Position elements
    algorithmComboBox->setBounds(0, 0, getParentWidth() * 0.9, 40);
    valueLabel->setBounds(0, 50, getParentWidth() * 0.9, 20);
}

void FeatureSlot::compute() {
    if(algorithm != nullptr){
        algorithm->compute();
        // Update output value for label
        float val = (int)(outputScalar * 100 + .5);
        outputValue = (float) (val / 100);
    }
}

FeatureSlot::Band FeatureSlot::getBand() {
    return band;
}

void FeatureSlot::setBand(Band band){
    this->band = band;
}

void FeatureSlot::initialiseAlgorithm(String algoStr) {
    // Big switcharoo for algorithm initialisation
    if(algoStr == "Loudness"){
        algorithm.reset(factory.create("Loudness"));
        algorithm->input("signal").set(*inputAudioBuffer);
        algorithm->output("loudness").set(outputScalar);
    }
    if(algoStr == "Spectral Centroid"){
        algorithm.reset(factory.create("SpectralCentroidTime", "sampleRate", (double) magicState.getPropertyAsValue("sampleRate").getValue()));
        algorithm->input("array").set(*inputAudioBuffer);
        algorithm->output("centroid").set(outputScalar);
    }

    // Initialise libmapper sensor
    // Remove previous signal from libmapper
    if(sensor != nullptr){
        libmapperDevice.remove_signal(*sensor);
    }
    // Add new signal
    std::string signalName = band == LOW ? "Low" : band == MID ? "Mid" : "High";
    signalName.append(" Slot ").append(to_string(slotNumber)).append(": ").append(algoStr.toStdString());

    sensor = make_unique<mapper::Signal>(libmapperDevice.add_output_signal(signalName, 1, 'f', 0, 0, 0));
}

void FeatureSlot::attachToParameter(const String& value, AudioProcessorValueTreeState& vts){
    algorithmComboBox->attachToParameter (value, vts);
}

void FeatureSlot::valueChanged (Value &value){
    int idx = static_cast<int>(value.getValue());
    if(idx == 0){
        algorithm.reset();
        return;
    }

    String algoName = featureSlotAlgorithmOptions.getReference(idx - 1);
    initialiseAlgorithm(algoName);
}

void FeatureSlot::setInputAudioBuffer(shared_ptr<vector<Real>> audioBuffer) {
    inputAudioBuffer = audioBuffer;
}

void FeatureSlot::setSlotNumber(int number) {
    this->slotNumber = number;
}
