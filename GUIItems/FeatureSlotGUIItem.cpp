//
// Created by Max on 21/06/2020.
//

#include "FeatureSlotGUIItem.h"
#include "../PluginProcessor.h"

FeatureSlotGUIItem::FeatureSlotGUIItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
        :foleys::GuiItem (builder, node),
        magicState(dynamic_cast<AudioPluginAudioProcessor*>(builder.getMagicState().getProcessor())->getMagicState()),
        lowBandSlots(dynamic_cast<AudioPluginAudioProcessor*>(builder.getMagicState().getProcessor())->getLowBandSlots()),
        midBandSlots(dynamic_cast<AudioPluginAudioProcessor*>(builder.getMagicState().getProcessor())->getMidBandSlots()),
        highBandSlots(dynamic_cast<AudioPluginAudioProcessor*>(builder.getMagicState().getProcessor())->getHighBandSlots())
        {
    if (auto* proc = dynamic_cast<AudioPluginAudioProcessor*>(builder.getMagicState().getProcessor()))
    {
        featureSlotGUI = make_unique<FeatureSlotGUI>(proc->getMagicState());
        addAndMakeVisible (featureSlotGUI.get());
    }
}

void FeatureSlotGUIItem::update() {
    auto val = getProperty("featureSlotParameter");
    if(!val.isVoid()){
        String valStr = val.toString();

        // Get current slot number
        char slot = valStr.toStdString().back();
        int slotNo = slot - '0';

        // Add feature slots to vector for access in processor
        if(valStr.contains("low")) {
            featureSlotGUI->registerValue(lowBandSlots[slotNo - 1]->getOutputValue());
        } else if(valStr.contains("mid")){
            featureSlotGUI->registerValue(midBandSlots[slotNo - 1]->getOutputValue());
        } else if(valStr.contains("high")){
            featureSlotGUI->registerValue(highBandSlots[slotNo - 1]->getOutputValue());
        }

        // Lastly, attach to value
        featureSlotGUI->attachToParameter(valStr, magicState.getValueTreeState());
    }
}

std::vector<foleys::SettableProperty> FeatureSlotGUIItem::getSettableProperties() const {
    juce::PopupMenu sliderTypes;
    for(int i = 0; i < featureSlotAlgorithmOptions.size(); i++){
        sliderTypes.addItem(i + 1, featureSlotAlgorithmOptions.getReference(i));
    }

    vector<foleys::SettableProperty> props;
    props.push_back({ configNode, "featureSlotParameter", foleys::SettableProperty::Choice, "-", sliderTypes });

    return props;
}