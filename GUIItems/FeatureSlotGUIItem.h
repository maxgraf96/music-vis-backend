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

/**
 * Wrapper class for the FeatureSlot GUI element into the GUI system
 */
class FeatureSlotGUIItem : public foleys::GuiItem {
public:
    FOLEYS_DECLARE_GUI_FACTORY (FeatureSlotGUIItem)

    FeatureSlotGUIItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node);

    /**
     * Pass-through method that connects the GUI item's wrapped FeatureSlot instance object to the state management
     */
    void update() override;

    /**
     * Method to populate the FeatureSlot's combobox choices
     * @return
     */
    std::vector<foleys::SettableProperty> getSettableProperties() const override;

    /**
     * Returns the wrapped FeatureSlotGUI instance object
     * @return
     */
    juce::Component* getWrappedComponent() override
    {
        return featureSlotGUI.get();
    }

private:
    // References to the FeatureSlotProcessors for each sub-band
    vector<unique_ptr<FeatureSlotProcessor>>& lowBandSlots;
    vector<unique_ptr<FeatureSlotProcessor>>& midBandSlots;
    vector<unique_ptr<FeatureSlotProcessor>>& highBandSlots;
    // Pointer to the wrapped FeatureSlotGUI instance object
    unique_ptr<FeatureSlotGUI> featureSlotGUI;
    // State management
    foleys::MagicProcessorState& magicState;
};


#endif //MUSIC_VIS_BACKEND_FEATURESLOTGUIITEM_H
