//
// Created by Max on 21/06/2020.
//
#pragma once
#ifndef MUSIC_VIS_BACKEND_FEATURESLOTGUIITEM_H
#define MUSIC_VIS_BACKEND_FEATURESLOTGUIITEM_H

#include "../Constants.h"
#include "../FeatureSlot/FeatureSlotProcessor.h"
#include "../foleys_gui_magic/foleys_gui_magic.h"
#include "../FeatureSlot/FeatureSlotGUI.h"

using namespace std;

class FeatureSlotGUIItem : public foleys::GuiItem {
public:
    FOLEYS_DECLARE_GUI_FACTORY (FeatureSlotGUIItem)

    FeatureSlotGUIItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node);

    void update() override;
    std::vector<foleys::SettableProperty> getSettableProperties() const override;

    juce::Component* getWrappedComponent() override
    {
        return featureSlotGUI.get();
    }

private:
    vector<unique_ptr<FeatureSlotProcessor>>& lowBandSlots;
    vector<unique_ptr<FeatureSlotProcessor>>& midBandSlots;
    vector<unique_ptr<FeatureSlotProcessor>>& highBandSlots;
    unique_ptr<FeatureSlotGUI> featureSlotGUI;
    foleys::MagicProcessorState& magicState;
};


#endif //MUSIC_VIS_BACKEND_FEATURESLOTGUIITEM_H
