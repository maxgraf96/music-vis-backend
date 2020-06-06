#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p, AudioProcessorValueTreeState& valueTreeState)
    : AudioProcessorEditor (&p), processorRef (p), spectrum(p.getSpectrumData()),
    spectralCentroid(p.getSpectralCentroid()), vts(valueTreeState)
{
    // Start timer and click every 50ms
    startTimer(50);

    // Initialise GUI elements
    cbNumberOfBands = make_unique<ComboBox>("cbNumberOfBands");

    // Add GUI elements
    addAndMakeVisible(*cbNumberOfBands);

    // Setup filter visualiser
    tooltip.reset(new TooltipWindow(this, 100));
    filterGraph.reset(new FilterGraph(p, valueTreeState, *tooltip));
    filterGraph->setBounds(705, 480, 282, 128);
    filterGraph->setTraceColour(Colour(0xff356931));
    addAndMakeVisible(*filterGraph);

    // Populate combo box
    cbNumberOfBands->addItemList(StringArray("1", "2", "3", "4"), 1);

    // Attach valueTreeState to GUI elements
    attachmentNumberOfBands = make_unique<ComboBoxAttachment>(vts, "numberOfBands", *cbNumberOfBands);

    // Set editor size last
    setSize (1024, 600);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    // Stop timer
    stopTimer();
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void AudioPluginAudioProcessorEditor::resized()
{
    cbNumberOfBands->setBounds(24, 24, 100, 24);
}

void AudioPluginAudioProcessorEditor::timerCallback() {
    // Just repaint for now
    repaint();
}
