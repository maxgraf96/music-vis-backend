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

    // Position elements
    algorithmComboBox->setBounds(0, 0, 120, 40);
    valueLabel->setBounds(0, 50, 120, 20);

    algorithmComboBox->addListener(this);
    algorithmComboBox->getSelectedIdAsValue().addListener(this);

    addAndMakeVisible(*algorithmComboBox);
    addAndMakeVisible(*valueLabel);

    setSize(200, 100);
}

FeatureSlot::~FeatureSlot() {
}

void FeatureSlot::paint(Graphics &) {

}

void FeatureSlot::resized() {

}

void FeatureSlot::compute() {
    if(algorithm != nullptr){
        algorithm->compute();
        // Update output value for label
        float val = (int)(outputScalar * 100 + .5);
        outputValue = (float) (val / 100);
    }
}

int FeatureSlot::getBand() {
    return band;
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
}

void FeatureSlot::attachToParameter(String value, AudioProcessorValueTreeState& vts){
    algorithmComboBox->attachToParameter (value, vts);
}

void FeatureSlot::comboBoxChanged(ComboBox* comboBoxThatHasChanged) {
    auto newVal = comboBoxThatHasChanged->getSelectedId();

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
