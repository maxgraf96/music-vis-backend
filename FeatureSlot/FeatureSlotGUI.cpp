//
// Created by Max on 16/06/2020.
//

#include "FeatureSlotGUI.h"

FeatureSlotGUI::FeatureSlotGUI(foleys::MagicProcessorState& ms)
: magicState(ms) {
    // Create the combobox for selecting the algorithm
    algorithmComboBox = make_unique<ComboBox>();
    // Create the label displaying the current output value
    valueLabel = make_unique<Label>("displayValue", "");

    // Add to component
    addAndMakeVisible(*algorithmComboBox);
    addAndMakeVisible(*valueLabel);

    // Set GUI size
    // TODO, Low prio: Make dynamic for better GUI responsiveness
    setSize(200, 100);
}

void FeatureSlotGUI::registerValue(Value& value){
    // Hook up label value to state management
    valueLabel->getTextValue().referTo(value);
}

void FeatureSlotGUI::attachToParameter(const String& value, AudioProcessorValueTreeState& vts){
    attachment.reset();
    if(value.isNotEmpty()){
        if(auto* parameter = magicState.getParameter(value)){
            algorithmComboBox->clear();
            // Populate combo box menu items with available algorithms
            algorithmComboBox->addItemList(featureSlotAlgorithmOptions, 1);
            attachment = magicState.createAttachment(value, *algorithmComboBox);
        }
    }
}

FeatureSlotGUI::~FeatureSlotGUI(){
    attachment.reset();
}

void FeatureSlotGUI::paint(Graphics &) {
    // Needed for integrity as JUCE component
}

void FeatureSlotGUI::resized() {
    // Position elements
    algorithmComboBox->setBounds(0, 0, getParentWidth() * 0.9, 30);
    valueLabel->setBounds(0, 35, getParentWidth() * 0.9, 20);
}