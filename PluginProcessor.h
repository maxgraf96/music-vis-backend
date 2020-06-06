#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>
#include "Utility.h"
#include <mapper/mapper_cpp.h>

using namespace juce;
using namespace std;
using namespace essentia;
using namespace essentia::standard;

//==============================================================================
class AudioPluginAudioProcessor  : public juce::AudioProcessor,
        private AudioProcessorValueTreeState::Listener
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

    // Getter for set of filters used to split signal into bands


    vector<Real>& getSpectrumData();
    Real& getSpectralCentroid();

private:
    // State management
    AudioProcessorValueTreeState valueTreeState;
    std::atomic<float>* paramNumberOfBands = nullptr;

    // Will contain copy of the JUCE audio buffer
    vector<Real> eAudioBuffer;
    // Will contain JUCE audio buffer after windowing
    vector<Real> windowedFrame;
    // Will contain the spectrum data
    vector<Real> spectrumData;
    Real spectralCentroid = 0.0f;
    Real estimatedPitch = 0.0f;
    Real pitchConfidence = 0.0f;

    unique_ptr<Algorithm> windowing;
    unique_ptr<Algorithm> spectrum;
    unique_ptr<Algorithm> mfcc;
    unique_ptr<Algorithm> specCentroid;
    unique_ptr<Algorithm> pitchYin;

    // Libmapper stuff
    unique_ptr<mapper::Device> dev;
    unique_ptr<mapper::Signal> sensor1;
    unique_ptr<mapper::Signal> sensor2;
    unique_ptr<mapper::Signal> pitchSensor;

    // Called if one of the parameters is changed, either through UI interaction or
    // manipulation from the host (such as automations)
    void parameterChanged(const String& parameterID, float newValue) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
    //==============================================================================
};
