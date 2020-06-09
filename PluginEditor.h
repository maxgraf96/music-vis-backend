#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "jucefiltergraph/FilterGraph.h"

using namespace std;

typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

//==============================================================================
class AudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer,
                                         private AudioProcessorValueTreeState::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&, AudioProcessorValueTreeState&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    AudioProcessorValueTreeState& vts;

    // Audio parameters
    atomic<float>* numberOfBands;

    vector<Real>& spectrum;
    Real& spectralCentroid;

    // GUI elements
    unique_ptr<ComboBox> cbNumberOfBands;
    // The filter visualisation component
    std::unique_ptr<FilterGraph> filterGraph;

    // Attachments for state management
    unique_ptr<ComboBoxAttachment> attachmentNumberOfBands;

    // Timer callback
    void timerCallback() override;

    // APVTS parameter changed callback
    void parameterChanged(const String& parameterID, float newValue) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
