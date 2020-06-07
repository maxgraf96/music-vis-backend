#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), valueTreeState(*this,
                         nullptr, // No undo manager
                         Identifier("music-vis-backend"),
                         {
                           std::make_unique<AudioParameterChoice>(
                                    "numberOfBands",
                                    "Number of Bands",
                                    StringArray("1", "2", "3"), // Support max 3 bands atm
                                    0
                            ),
                            std::make_unique<AudioParameterFloat>(
                                    "lowpassCutoff",
                                    "Lowpass Filter Cutoff",
                                    20.0f,
                                    20000.0f,
                                    3000.0f
                                    ),
                           std::make_unique<AudioParameterFloat>(
                                   "highpassCutoff",
                                   "Highpass Filter Cutoff",
                                   20.0f,
                                   20000.0f,
                                   5000.0f
                           )
                         })
{
    // Initialise listeners for parameters
    valueTreeState.addParameterListener("numberOfBands", this);
    valueTreeState.addParameterListener("lowpassCutoff", this);
    valueTreeState.addParameterListener("highpassCutoff", this);

    // Hook up parameters to values
    paramNumberOfBands = valueTreeState.getRawParameterValue("numberOfBands");
    paramLowpassCutoff.referTo(valueTreeState.getParameterAsValue("lowpassCutoff"));
    auto test = paramLowpassCutoff.getValue();
    paramHighpassCutoff = valueTreeState.getRawParameterValue("highpassCutoff");

    // Setup libmapper
    dev = make_unique<mapper::Device>("test");
    sensor1 = make_unique<mapper::Signal>(dev->add_output_signal("sensor1", 1, 'f', nullptr, nullptr, nullptr));
    sensor2 = make_unique<mapper::Signal>(dev->add_output_signal("sensor2", 128, 'f', 0, 0, 0));
    pitchSensor = make_unique<mapper::Signal>(dev->add_output_signal("pitchSensor", 1, 'f', 0, 0, 0));
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Clear essentia audiobuffer
    eAudioBuffer.clear();

    auto* reader = buffer.getReadPointer(0);
    for (int i = 0; i < buffer.getNumSamples(); i++){
        eAudioBuffer.push_back(reader[i]);
    }

    windowing->compute();
    spectrum->compute();
    specCentroid->compute();
    pitchYin->compute();

    // Poll libmapper device
    dev->poll();

    // Send spectral centroid to libmapper
    sensor1->update(spectralCentroid);
    sensor2->update(spectrumData);
    pitchSensor->update(estimatedPitch);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialise essentia
    essentia::init();
    // Create algo pool
    Pool pool;

    // Create algorithms
    standard::AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
    windowing.reset(factory.create("Windowing", "type", "blackmanharris62"));
    spectrum.reset(factory.create("Spectrum"));
    mfcc.reset(factory.create("MFCC"));
    specCentroid.reset(factory.create("SpectralCentroidTime", "sampleRate", sampleRate));
    pitchYin.reset(factory.create("PitchYin", "sampleRate", sampleRate, "frameSize", samplesPerBlock));

    // Connect algorithms
    windowing->input("frame").set(eAudioBuffer);
    windowing->output("frame").set(windowedFrame);
    spectrum->input("frame").set(windowedFrame);
    spectrum->output("spectrum").set(spectrumData);

    // Pitch detection
    pitchYin->input("signal").set(eAudioBuffer);
    pitchYin->output("pitch").set(estimatedPitch);
    pitchYin->output("pitchConfidence").set(pitchConfidence);

    specCentroid->input("array").set(eAudioBuffer);
    specCentroid->output("centroid").set(spectralCentroid);

}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // Shutdown essentia
    essentia::shutdown();
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this, valueTreeState);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Store state when closing plugin
    const auto state = valueTreeState.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore saved state
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(valueTreeState.state.getType()))
            valueTreeState.replaceState(ValueTree::fromXml(*xmlState));

    // Set filter cutoff frequencies
    paramLowpassCutoff = paramLowpassCutoff.getValue();
    auto test = paramLowpassCutoff.getValue();
    for (int i = 0; i < lowpassFilters.size(); i++) {
        lowpassFilters[i].coefficients = dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), paramLowpassCutoff.getValue(), SQRT_2_OVER_2);
        highpassFilters[i].coefficients = dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), *paramHighpassCutoff, SQRT_2_OVER_2);
    }
}

vector <Real> &AudioPluginAudioProcessor::getSpectrumData() {
    return spectrumData;
}

Real &AudioPluginAudioProcessor::getSpectralCentroid() {
    return spectralCentroid;
}

void AudioPluginAudioProcessor::parameterChanged(const String &parameterID, float newValue) {
    if(parameterID == "lowpassCutoff"){
        paramLowpassCutoff = newValue;
        // Set filter cutoff frequencies
        for (auto & lowpassFilter : lowpassFilters) {
            lowpassFilter.coefficients = dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), newValue, SQRT_2_OVER_2);
        }
    }
    if(parameterID == "highpassCutoff"){
        // Set filter cutoff frequencies
        for (auto & highpassFilter : highpassFilters) {
            highpassFilter.coefficients = dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), newValue, SQRT_2_OVER_2);
        }
    }
}

array<dsp::IIR::Filter<float>, 2> &AudioPluginAudioProcessor::getLowpassFilters() {
    return lowpassFilters;
}

array<dsp::IIR::Filter<float>, 2> &AudioPluginAudioProcessor::getHighpassFilters() {
    return highpassFilters;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
