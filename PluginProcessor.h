#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>
#include "Utility.h"
#include <mapper/mapper_cpp.h>
#include "foleys_gui_magic/foleys_gui_magic.h"
#include "BinaryData.h"
#include "FeatureSlot/FeatureSlotProcessor.h"
#include "FeatureSlot/FeatureSlotGUI.h"

using namespace juce;
using namespace std;
using namespace essentia;
using namespace essentia::standard;

//==============================================================================
class AudioPluginAudioProcessor  : public juce::AudioProcessor,
private AudioProcessorValueTreeState::Listener, Timer
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

    // Pick up changes to the track at which the plugin is located
    void updateTrackProperties(const TrackProperties& properties) override;

    vector<Real>& getSpectrumData();
    Real& getSpectralCentroid();

    // Getters for filters - used in FilterGraph
    array<dsp::IIR::Filter<float>, 2>& getLowpassFilters();
    array<dsp::IIR::Filter<float>, 2>& getHighpassFilters();

private:
    // State management
    AudioProcessorValueTreeState valueTreeState;
    // Number of audio bands to which to split the main signal
    // Allows for separate processing of high, mid and low-end
    atomic<float>* paramNumberOfBands = nullptr;
    // Cutoff frequency for the lowpass filter
    Value paramLowpassCutoff;
    // Cutoff frequency for the highpass filter
    Value paramHighpassCutoff;
    atomic<float>* paramLowSolo = nullptr;
    atomic<float>* paramMidSolo = nullptr;
    atomic<float>* paramHighSolo = nullptr;
    vector<atomic<float>*> autoParams;

    // NB: The cutoff frequencies for the mid-band are calculated from the high- and low band filters respectively
    // Main filters: 2 for lowpass, 2 for highpass
    array<dsp::IIR::Filter<float>, 2> lowpassFilters;
    array<dsp::IIR::Filter<float>, 2> highpassFilters;

    // Buffers for low, mid and high bands
    unique_ptr<AudioBuffer<float>> lowBuffer;
    unique_ptr<AudioBuffer<float>> midBuffer;
    unique_ptr<AudioBuffer<float>> highBuffer;

    // Helper function to determine whether any band is currently solo'ed
    bool noSolo();

    // PluginGUIMagic stuff
    foleys::MagicProcessorState magicState { *this, valueTreeState };
    // filtergraph component registration
    void registerFilterGraph(foleys::MagicGUIBuilder& builder, AudioPluginAudioProcessor* processor);
    void registerFeatureSlotGUI(foleys::MagicGUIBuilder &builder, AudioPluginAudioProcessor *processor);

    Label* spectralCentroidLabel;

    // Necessary for enabling tooltips
    std::unique_ptr<TooltipWindow> tooltip;

    // Values estimated by Essentia are marked with an "e" prefix
    // Will contain copy of the global JUCE audio buffer (not subdivided into bands)
    // This buffer is used in the calculation of global audio features
    vector<Real> eGlobalAudioBuffer;
    // Low, mid and high band buffers
    vector<Real>   eLowAudioBuffer;
    vector<Real> eMidAudioBuffer;
    vector<Real> eHighAudioBuffer;

    // Will contain JUCE audio buffer after windowing
    vector<Real> windowedFrame;
    // Will contain the spectrum data
    vector<Real> eSpectrumData;
    Real eSpectralCentroid = 0.0f;
    Real ePitchYIN = 0.0f;
    Real ePitchConfidence = 0.0f;
    Real eLoudness = 0.0f;

    // Essentia algorithms are marked by an "a" prefix
    unique_ptr<Algorithm> aWindowing;
    unique_ptr<Algorithm> aSpectrum;
    unique_ptr<Algorithm> aMFCC;
    unique_ptr<Algorithm> aSpectralCentroid;
    unique_ptr<Algorithm> aPitchYIN;
    unique_ptr<Algorithm> aLoudness;

    // Libmapper stuff
    void libmapperSetup(const string& deviceName);
    unique_ptr<mapper::Device> libmapperDevice;
    unique_ptr<mapper::Signal> sensorSpectralCentroid;
    unique_ptr<mapper::Signal> sensorSpectrum;
    unique_ptr<mapper::Signal> sensorPitchYIN;
    unique_ptr<mapper::Signal> sensorLoudness;
    vector<unique_ptr<mapper::Signal>> sensorsAutomatables;

    // Feature slots
    vector<unique_ptr<FeatureSlotProcessor>> lowBandSlots;
    vector<unique_ptr<FeatureSlotProcessor>> midBandSlots;
    vector<unique_ptr<FeatureSlotProcessor>> highBandSlots;
    int slotCounter = 0;

    // Called if one of the parameters is changed, either through UI interaction or
    // manipulation from the host (such as automations)
    void parameterChanged(const String& parameterID, float newValue) override;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
    //==============================================================================
};

// Identifiers for GUI interaction
static Identifier SPECTRAL_CENTROID_ID = "spectralCentroidValue";
static Identifier PITCH_YIN_ID = "pitchYINValue";
static Identifier MID_MAX_WIDTH_ID = "midMaxWidth";
static Identifier MULTIBAND_ENABLED_ID = "multiBandEnabled";
static Identifier MIDBAND_ENABLED_ID = "midBandEnabled";
static Identifier LOUDNESS_ID = "loudnessValue";

