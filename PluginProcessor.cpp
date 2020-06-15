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
                         ),
                         // Sub band algorithm slot selectors
                         // Low band
                           make_unique<AudioParameterChoice>(
                                   "lowSlot1",
                                   "Low Band Slot 1 Algorithm",
                                   featureSlotAlgorithmOptions,
                                   0
                           ),
                           make_unique<AudioParameterChoice>(
                                   "lowSlot2",
                                   "Low Band Slot 2 Algorithm",
                                   featureSlotAlgorithmOptions,
                                   0
                           ),
                           // Mid band
                           make_unique<AudioParameterChoice>(
                                   "midSlot1",
                                   "Mid Band Slot 1 Algorithm",
                                   featureSlotAlgorithmOptions,
                                   0
                           ),
                           make_unique<AudioParameterChoice>(
                                   "midSlot2",
                                   "Mid Band Slot 2 Algorithm",
                                   featureSlotAlgorithmOptions,
                                   0
                           ),
                           // High band
                           make_unique<AudioParameterChoice>(
                                   "highSlot1",
                                   "High Band Slot 1 Algorithm",
                                   featureSlotAlgorithmOptions,
                                   0
                           ),
                           make_unique<AudioParameterChoice>(
                                   "highSlot2",
                                   "High Band Slot 2 Algorithm",
                                   featureSlotAlgorithmOptions,
                                   0
                           ),
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

    // Initialise essentia
    essentia::init();

    // Setup libmapper
    libmapperDevice = make_unique<mapper::Device>("test");
    sensorSpectralCentroid = make_unique<mapper::Signal>(libmapperDevice->add_output_signal("spectralCentroid", 1, 'f', nullptr, nullptr, nullptr));
    sensorSpectrum = make_unique<mapper::Signal>(libmapperDevice->add_output_signal("spectrum", 128, 'f', 0, 0, 0));
    sensorPitchYIN = make_unique<mapper::Signal>(libmapperDevice->add_output_signal("pitchYIN", 1, 'f', 0, 0, 0));
    sensorLoudness = make_unique<mapper::Signal>(libmapperDevice->add_output_signal("loudness", 1, 'f', 0, 0, 0));

    // Start timer for GUI updates
    startTimer(100);

    // Setup sub-band buffers
    eLowAudioBuffer = make_shared<vector<Real>>();
    eMidAudioBuffer = make_shared<vector<Real>>();
    eHighAudioBuffer = make_shared<vector<Real>>();
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
    eGlobalAudioBuffer.clear();

    auto* reader = buffer.getReadPointer(0);
    for (int i = 0; i < buffer.getNumSamples(); i++){
        eGlobalAudioBuffer.push_back(reader[i]);
    }

    aWindowing->compute();
    aSpectrum->compute();
    aSpectralCentroid->compute();
    aPitchYIN->compute();
    aLoudness->compute();

    // Poll libmapper device
    libmapperDevice->poll();

    // Send spectral centroid to libmapper
    sensorSpectralCentroid->update(eSpectralCentroid);
    sensorSpectrum->update(eSpectrumData);
    sensorPitchYIN->update(ePitchYIN);
    sensorLoudness->update(eLoudness);

    // Additional multiband processing (if more than 1 band is seleted)
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

        // Send to sub-bands and process
        // Clear essentia sub band buffers
        eLowAudioBuffer->clear();
        eMidAudioBuffer->clear();
        eHighAudioBuffer->clear();

        auto* lowReader = lowBuffer->getReadPointer(0);
        auto* midReader = midBuffer->getReadPointer(0);
        auto* highReader = highBuffer->getReadPointer(0);

        for (int i = 0; i < buffer.getNumSamples(); i++){
            eLowAudioBuffer->emplace_back(lowReader[i]);
            eMidAudioBuffer->emplace_back(midReader[i]);
            eHighAudioBuffer->emplace_back(highReader[i]);
        }

        // 2 bands (low and high) => ignore mid band
        for(auto& featureSlot : lowBandSlots){
            featureSlot->compute();
        }
        for(auto& featureSlot : highBandSlots){
            featureSlot->compute();
        }
        // Also process mid-band if three bands are selected
        if(*paramNumberOfBands == 2.0f){
            for(auto& featureSlot : midBandSlots){
                featureSlot->compute();
            }
        }

        // Clear main buffer
        buffer.clear();

        // Playback
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
    // Reinitialise essentia if not initialised
    if(!essentia::isInitialized()){
        essentia::init();
    }

    // Store sr in state management
    magicState.getPropertyAsValue("sampleRate").setValue(sampleRate);

    // Create algorithms
    standard::AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
    aWindowing.reset(factory.create("Windowing", "type", "blackmanharris62"));
    aSpectrum.reset(factory.create("Spectrum"));
    aMFCC.reset(factory.create("MFCC"));
    aSpectralCentroid.reset(factory.create("SpectralCentroidTime", "sampleRate", sampleRate));
    aPitchYIN.reset(factory.create("PitchYin", "sampleRate", sampleRate, "frameSize", samplesPerBlock));
    aLoudness.reset(factory.create("Loudness"));

    // Connect algorithms
    aWindowing->input("frame").set(eGlobalAudioBuffer);
    aWindowing->output("frame").set(windowedFrame);
    aSpectrum->input("frame").set(windowedFrame);
    aSpectrum->output("spectrum").set(eSpectrumData);

    // Pitch detection
    aPitchYIN->input("signal").set(eGlobalAudioBuffer);
    aPitchYIN->output("pitch").set(ePitchYIN);
    aPitchYIN->output("pitchConfidence").set(ePitchConfidence);

    // Spectral centroid
    aSpectralCentroid->input("array").set(eGlobalAudioBuffer);
    aSpectralCentroid->output("centroid").set(eSpectralCentroid);

    // Loudness
    aLoudness->input("signal").set(eGlobalAudioBuffer);
    aLoudness->output("loudness").set(eLoudness);

    // Setup band buffers
    lowBuffer = make_unique<AudioBuffer<float>>(2, samplesPerBlock);
    midBuffer = make_unique<AudioBuffer<float>>(2, samplesPerBlock);
    highBuffer = make_unique<AudioBuffer<float>>(2, samplesPerBlock);
}

bool AudioPluginAudioProcessor::noSolo() {
    return *paramLowSolo == 0.0f && *paramMidSolo == 0.0f && *paramHighSolo == 0.0f;
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    lowBandSlots.clear();
    midBandSlots.clear();
    highBandSlots.clear();
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
    registerFeatureSlot(*builder, this);
    magicState.setLastEditorSize(1200, 1024);

    return new foleys::MagicPluginEditor(magicState, BinaryData::musicvisbackend_xml, BinaryData::musicvisbackend_xmlSize, std::move(builder));
//    return new foleys::MagicPluginEditor (magicState, MyBinaryData::getMagicXML(), MyBinaryData::getMagicXMLSize(), std::move (builder));
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    magicState.getStateInformation (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Load plugin state from disk
    magicState.setStateInformation (data, sizeInBytes, getActiveEditor());

    // Set filter cutoff frequencies
    paramLowpassCutoff = paramLowpassCutoff.getValue();
    paramHighpassCutoff = paramHighpassCutoff.getValue();
    for (int i = 0; i < lowpassFilters.size(); i++) {
        lowpassFilters[i].coefficients = dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), paramLowpassCutoff.getValue(), SQRT_2_OVER_2);
        highpassFilters[i].coefficients = dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), paramHighpassCutoff.getValue(), SQRT_2_OVER_2);
    }

    // Show / hide mid band
    magicState.getPropertyAsValue(MIDBAND_ENABLED_ID.toString()).setValue(*paramNumberOfBands == 2.0f);
}

vector <Real> &AudioPluginAudioProcessor::getSpectrumData() {
    return eSpectrumData;
}

Real &AudioPluginAudioProcessor::getSpectralCentroid() {
    return eSpectralCentroid;
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
        magicState.getPropertyAsValue(MULTIBAND_ENABLED_ID.toString()).setValue(newValue > 0.0f);
        magicState.getPropertyAsValue(MIDBAND_ENABLED_ID.toString()).setValue(newValue == 2.0f);

        // If 2 bands are selected snap highpass cutoff value to lowpass cutoff value
        if(newValue == 1.0f){
            magicState.getValueTreeState().getParameterAsValue("highpassCutoff").setValue(paramLowpassCutoff.getValue());
            for (auto & filter : highpassFilters) {
                filter.coefficients = dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), paramLowpassCutoff.getValue(), SQRT_2_OVER_2);
            }
        }
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
        return make_unique<FilterGraph>(*processor, processor->magicState.getValueTreeState(), *processor->tooltip);
    });
}

void AudioPluginAudioProcessor::registerFeatureSlot(foleys::MagicGUIBuilder& builder, AudioPluginAudioProcessor* processor) {
    builder.registerFactory ("FeatureSlot", [processor](const ValueTree&)
    {
        return make_unique<FeatureSlot>(*processor->libmapperDevice, processor->magicState);
    });
    // Add settable properties
    builder.addSettableProperty ("FeatureSlot",
                                std::make_unique<foleys::SettableChoiceProperty>
                                        (foleys::IDs::featureSlotParameter,
                                         [&] (juce::Component* component, juce::var value, const juce::NamedValueSet&)
                                         {
                                             if (auto* featureSlot = dynamic_cast<FeatureSlot*>(component)){
                                                 String valStr = value.toString();
                                                 featureSlot->attachToParameter(valStr, magicState.getValueTreeState());

                                                 // Get current slot number
                                                 char slot = valStr.toStdString().back();
                                                 int slotNo = slot - '0';
                                                 featureSlot->setSlotNumber(slotNo);

                                                 // Add feature slots to vector for access in processor
                                                 if(valStr.contains("low")
                                                    && find(lowBandSlots.begin(), lowBandSlots.end(), featureSlot) == lowBandSlots.end()){
                                                     featureSlot->setInputAudioBuffer(eLowAudioBuffer);
                                                     featureSlot->setBand(FeatureSlot::LOW);
                                                     lowBandSlots.emplace_back(featureSlot);
                                                 } else if(valStr.contains("mid")
                                                       && find(midBandSlots.begin(), midBandSlots.end(), featureSlot) == midBandSlots.end()){
//                                                     featureSlot->setInputAudioBuffer(eMidAudioBuffer);
                                                     featureSlot->setBand(FeatureSlot::MID);
                                                     midBandSlots.emplace_back(featureSlot);
                                                 } else if(valStr.contains("high")
                                                       && find(highBandSlots.begin(), highBandSlots.end(), featureSlot) == highBandSlots.end()){
                                                     featureSlot->setInputAudioBuffer(eHighAudioBuffer);
                                                     featureSlot->setBand(FeatureSlot::HIGH);
                                                     highBandSlots.emplace_back(featureSlot);
                                                 }
                                             }
                                         },
                                         juce::String(),
                                         foleys::SettableProperty::Parameter));
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
    // Display current spectral centroid
    magicState.getPropertyAsValue(SPECTRAL_CENTROID_ID.toString()).setValue(roundToInt(eSpectralCentroid));
    // Only display pitch if confidence is greater than chance
    auto pitchValue = ePitchConfidence > 0.5 ? ePitchYIN : -1;
    magicState.getPropertyAsValue(PITCH_YIN_ID.toString()).setValue(roundToInt(pitchValue));
    // Display current loudness
    magicState.getPropertyAsValue(LOUDNESS_ID.toString()).setValue(roundToInt(eLoudness));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
