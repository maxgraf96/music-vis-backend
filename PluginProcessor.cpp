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
                           make_unique<AudioParameterChoice>(
                                    "numberOfBands",
                                    "Number of Bands",
                                    StringArray("1", "2", "3"), // Support max 3 bands atm
                                    0
                            ),
                            make_unique<AudioParameterFloat>(
                                    "lowpassCutoff",
                                    "Lowpass Filter Cutoff",
                                    20.0f,
                                    20000.0f,
                                    3000.0f
                                    ),
                           make_unique<AudioParameterFloat>(
                                   "highpassCutoff",
                                   "Highpass Filter Cutoff",
                                   20.0f,
                                   20000.0f,
                                   5000.0f
                           ),
                           make_unique<AudioParameterBool>(
                                   "lowSolo",
                                   "Low Band Solo",
                                   false
                                   ),
                           make_unique<AudioParameterBool>(
                                   "midSolo",
                                   "Mid Band Solo",
                                   false
                           ),
                           make_unique<AudioParameterBool>(
                                 "highSolo",
                                 "High Band Solo",
                                 false
                         )
                         })
{
    // Initialise listeners for parameters
    magicState.getValueTreeState().addParameterListener("numberOfBands", this);
    magicState.getValueTreeState().addParameterListener("lowpassCutoff", this);
    magicState.getValueTreeState().addParameterListener("highpassCutoff", this);
    magicState.getValueTreeState().addParameterListener("lowSolo", this);
    magicState.getValueTreeState().addParameterListener("midSolo", this);
    magicState.getValueTreeState().addParameterListener("highSolo", this);

    // Hook up parameters to values
    paramNumberOfBands = magicState.getValueTreeState().getRawParameterValue("numberOfBands");
    paramLowpassCutoff.referTo(magicState.getValueTreeState().getParameterAsValue("lowpassCutoff"));
    paramHighpassCutoff.referTo(magicState.getValueTreeState().getParameterAsValue("highpassCutoff"));
    paramLowSolo = magicState.getValueTreeState().getRawParameterValue("lowSolo");
    paramMidSolo = magicState.getValueTreeState().getRawParameterValue("midSolo");
    paramHighSolo = magicState.getValueTreeState().getRawParameterValue("highSolo");

    // Setup libmapper
    dev = make_unique<mapper::Device>("test");
    sensor1 = make_unique<mapper::Signal>(dev->add_output_signal("sensor1", 1, 'f', nullptr, nullptr, nullptr));
    sensor2 = make_unique<mapper::Signal>(dev->add_output_signal("sensor2", 128, 'f', 0, 0, 0));
    pitchSensor = make_unique<mapper::Signal>(dev->add_output_signal("pitchSensor", 1, 'f', 0, 0, 0));

    // Start timer for GUI updates
    startTimer(100);
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

    // Multiband processing (if enabled)
    if(*paramNumberOfBands > 0.0f){
        lowBuffer->clear();
        midBuffer->clear();
        highBuffer->clear();

        auto numSamples = buffer.getNumSamples();

        // Loop over channels and perform filtering
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            // Copy samples to buffers
            lowBuffer->copyFrom(channel, 0, buffer, channel, 0, numSamples);
            midBuffer->copyFrom(channel, 0, buffer, channel, 0, numSamples);
            highBuffer->copyFrom(channel, 0, buffer, channel, 0, numSamples);

            // Perform filtering
            auto* lowWriter = lowBuffer->getWritePointer(channel);
            auto* highWriter = highBuffer->getWritePointer(channel);
            for (int sample = 0; sample < numSamples; sample++){
                lowWriter[sample] = lowpassFilters[channel].processSample(lowWriter[sample]);
                highWriter[sample] = highpassFilters[channel].processSample(highWriter[sample]);
            }

            // Calculate mid band by subtracting low and high band from input signal
            midBuffer->addFrom(channel, 0, lowBuffer->getReadPointer(channel), numSamples, -1.0f);
            midBuffer->addFrom(channel, 0, highBuffer->getReadPointer(channel), numSamples, -1.0f);
        }

        // Clear main buffer
        buffer.clear();

        // If no channel is soloed, play all streams back
        if (noSolo()) {
            for (int channel = 0; channel < totalNumOutputChannels; channel++) {
                buffer.addFrom(channel, 0, *lowBuffer, channel, 0, numSamples);
                buffer.addFrom(channel, 0, *midBuffer, channel, 0, numSamples);
                buffer.addFrom(channel, 0, *highBuffer, channel, 0, numSamples);
            }
        }
        // Otherwise play only from soloed bands
        else {
            for (int channel = 0; channel < totalNumOutputChannels; channel++) {
                if(*paramLowSolo > 0.0f){
                    buffer.addFrom(channel, 0, *lowBuffer, channel, 0, numSamples);
                }
                if(*paramMidSolo > 0.0f){
                    buffer.addFrom(channel, 0, *midBuffer, channel, 0, numSamples);
                }
                if(*paramHighSolo > 0.0f){
                    buffer.addFrom(channel, 0, *highBuffer, channel, 0, numSamples);
                }
            }
        }

    }
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

    // Setup buffers
    lowBuffer = make_unique<AudioBuffer<float>>(2, samplesPerBlock);
    midBuffer = make_unique<AudioBuffer<float>>(2, samplesPerBlock);
    highBuffer = make_unique<AudioBuffer<float>>(2, samplesPerBlock);
}

bool AudioPluginAudioProcessor::noSolo() {
    return *paramLowSolo == 0.0f && *paramMidSolo == 0.0f && *paramHighSolo == 0.0f;
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
    // MAGIC GUI: we create our custom builder instance here, that will be available for all factories we add
    auto builder = std::make_unique<foleys::MagicGUIBuilder>(&magicState);
    builder->registerJUCEFactories();

    registerFilterGraph(*builder, this);
    magicState.setLastEditorSize(1200, 1024);

    return new foleys::MagicPluginEditor (magicState, MyBinaryData::getMagicXML(), MyBinaryData::getMagicXMLSize(), std::move (builder));
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    magicState.getStateInformation (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    magicState.setStateInformation (data, sizeInBytes, getActiveEditor());

    // Set filter cutoff frequencies
    paramLowpassCutoff = paramLowpassCutoff.getValue();
    paramHighpassCutoff = paramHighpassCutoff.getValue();
    auto test = paramLowpassCutoff.getValue();
    auto testhi = paramHighpassCutoff.getValue();
    for (int i = 0; i < lowpassFilters.size(); i++) {
        lowpassFilters[i].coefficients = dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), paramLowpassCutoff.getValue(), SQRT_2_OVER_2);
        highpassFilters[i].coefficients = dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), paramHighpassCutoff.getValue(), SQRT_2_OVER_2);
    }

    // Show / hide mid band
    magicState.getPropertyAsValue(MID_BAND_VISIBLE_ID.toString()).setValue(*paramNumberOfBands == 2.0f);
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

        // Also update highpassCutoff if 2 bands are selected
        if(*paramNumberOfBands == 1.0f){
            magicState.getValueTreeState().getParameterAsValue("highpassCutoff").setValue(newValue);
            for (auto & filter : highpassFilters) {
                filter.coefficients = dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), newValue, SQRT_2_OVER_2);
            }
        }
    }
    if(parameterID == "highpassCutoff"){
        paramHighpassCutoff = newValue;
        // Set filter cutoff frequencies
        for (auto & highpassFilter : highpassFilters) {
            highpassFilter.coefficients = dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), newValue, SQRT_2_OVER_2);
        }

        // Also update lowpassCutoff if 2 bands are selected
        if(*paramNumberOfBands == 1.0f){
            magicState.getValueTreeState().getParameterAsValue("lowpassCutoff").setValue(newValue);
            for (auto & filter : lowpassFilters) {
                filter.coefficients = dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), newValue, SQRT_2_OVER_2);
            }
        }
    }
    if(parameterID == "numberOfBands"){
        magicState.getPropertyAsValue(MID_BAND_VISIBLE_ID.toString()).setValue(newValue == 2.0f);
        magicState.getPropertyAsValue(MULTIBAND_ENABLED.toString()).setValue(newValue > 0.0f);
    }
}

array<dsp::IIR::Filter<float>, 2> &AudioPluginAudioProcessor::getLowpassFilters() {
    return lowpassFilters;
}

array<dsp::IIR::Filter<float>, 2> &AudioPluginAudioProcessor::getHighpassFilters() {
    return highpassFilters;
}

void AudioPluginAudioProcessor::registerFilterGraph(foleys::MagicGUIBuilder& builder, AudioPluginAudioProcessor* processor) {
    tooltip = make_unique<TooltipWindow>(this->getActiveEditor(), 100);

    builder.registerFactory ("FilterGraph", [processor](const ValueTree&)
    {
        return std::make_unique<FilterGraph>(*processor, processor->magicState.getValueTreeState(), *processor->tooltip);
    });
}

Component *AudioPluginAudioProcessor::getChildComponentWithID(Component *parent, String id) {
    for (int i = 0; i < parent->getNumChildComponents(); i++)
    {
        auto* childComp = parent->getChildComponent(i);
        DBG(childComp->getComponentID());

        if (childComp->getComponentID() == id)
            return childComp;

        if (auto c = getChildComponentWithID (childComp, id))
            return c;
    }

    return nullptr;
}

void AudioPluginAudioProcessor::timerCallback() {
    magicState.getPropertyAsValue(SPECTRAL_CENTROID_ID.toString()).setValue(roundToInt(spectralCentroid));
    magicState.getPropertyAsValue(PITCH_YIN_ID.toString()).setValue(roundToInt(estimatedPitch));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
