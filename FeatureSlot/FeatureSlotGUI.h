//
// Created by Max on 16/06/2020.
//

#ifndef MUSIC_VIS_BACKEND_FEATURESLOTGUI_H
#define MUSIC_VIS_BACKEND_FEATURESLOTGUI_H
#include <juce_audio_processors/juce_audio_processors.h>
#include "../foleys_gui_magic/foleys_gui_magic.h"

using namespace std;
using namespace juce;

class FeatureSlotGUI : public Component {
public:
    explicit FeatureSlotGUI(foleys::MagicProcessorState&);
    ~FeatureSlotGUI();
    void attachToParameter(const String& value, AudioProcessorValueTreeState& vts);
    void paint (Graphics&) override;
    void resized() override;

    void registerValue(Value& value);

private:
    // State management
    foleys::MagicProcessorState& magicState;

    // Display stuff
    unique_ptr<foleys::AttachableComboBox> algorithmComboBox;
    unique_ptr<Label> valueLabel;
};


#endif //MUSIC_VIS_BACKEND_FEATURESLOTGUI_H
