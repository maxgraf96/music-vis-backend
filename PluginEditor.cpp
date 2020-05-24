#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), spectrum(p.getSpectrumData()), spectralCentroid(p.getSpectralCentroid())
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1024, 600);

    // Start timer and click every 50ms
    startTimer(50);
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

    // Print first 20 magnitudes
    for(int i = 0; i < 20; i++){
        auto current = spectrum[i];
        g.drawSingleLineText(juce::String(current), 24, 24 + 10 * i);
    }

    // Draw spectral centroid
    g.drawRoundedRectangle(getWidth() / 2,
            mapFloat(spectralCentroid, 0.0f, 5000.0f, getHeight() - 48, 48),
            24,
            24,
            12,
            2.0);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void AudioPluginAudioProcessorEditor::timerCallback() {
    // Just repaint for now
    repaint();
}
