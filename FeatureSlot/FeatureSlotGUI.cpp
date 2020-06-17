//
// Created by Max on 16/06/2020.
//

#include "FeatureSlotGUI.h"

FeatureSlotGUI::FeatureSlotGUI(foleys::MagicProcessorState& ms)
: magicState(ms) {
    algorithmComboBox = make_unique<foleys::AttachableComboBox>();
    valueLabel = make_unique<Label>("displayValue", "");

    // Add to parent
    addAndMakeVisible(*algorithmComboBox);
    addAndMakeVisible(*valueLabel);

    setSize(200, 100);
}

void FeatureSlotGUI::registerValue(Value& value){
    valueLabel->getTextValue().referTo(value);
}

void FeatureSlotGUI::attachToParameter(const String& value, AudioProcessorValueTreeState& vts){
    algorithmComboBox->attachToParameter (value, vts);
}

FeatureSlotGUI::~FeatureSlotGUI(){
}

void FeatureSlotGUI::paint(Graphics &) {

}

void FeatureSlotGUI::resized() {
    // Position elements
    algorithmComboBox->setBounds(0, 0, getParentWidth() * 0.9, 30);
    valueLabel->setBounds(0, 35, getParentWidth() * 0.9, 20);
}