#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>
#include "Utility.h"

using namespace std;
using namespace essentia;
using namespace essentia::standard;

//==============================================================================
class AudioPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    vector<Real>& getSpectrumData();
    Real& getSpectralCentroid();

private:
    // Will contain copy of the JUCE audio buffer
    vector<Real> eAudioBuffer;
    // Will contain JUCE audio buffer after windowing
    vector<Real> windowedFrame;
    // Will contain the spectrum data
    vector<Real> spectrumData;
    Real spectralCentroid = 0.0f;

    unique_ptr<Algorithm> windowing;
    unique_ptr<Algorithm> spectrum;
    unique_ptr<Algorithm> mfcc;
    unique_ptr<Algorithm> specCentroid;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
    //==============================================================================
};
