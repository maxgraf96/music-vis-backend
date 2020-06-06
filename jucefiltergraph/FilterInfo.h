/*
  ==============================================================================

    FilterInfo.h
    
    Wrapper for filters and their responses.
	Adapted from Sean Enderby's implementation: https://sourceforge.net/projects/jucefiltergraph/

  ==============================================================================
*/

#include <juce_dsp/juce_dsp.h>
#include "../PluginProcessor.h"
#include <complex>
#include <vector>

using namespace std;
using namespace juce;

#ifndef double_E
#define double_E 2.71828183
#endif
//====================================================================================
enum FilterType
{
	LowPass,
	HighPass
};
    
enum BandType
{
    LowShelf,
    HighShelf,
    Peak
};

//==============================================================================
class FilterResponse
{
public:
    FilterResponse (double magnitudeInit, double phaseInit);
    ~FilterResponse();
    
    double magnitudeValue, phaseValue;
};

//============================================================================
class FilterInfo
{
public:    
    FilterInfo(AudioPluginAudioProcessor&, AudioProcessorValueTreeState&);
    ~FilterInfo();
    
    void setSampleRate (double sampleRate);
    void setGain (double gain);
    void updateFilter(float cutoff, float q);

	// This method was adapted for modern JUCE versions
	// It calls the getMagnitudeForFrequency(...) and getPhaseForFrequency(...)
	// methods to generate the paths used to draw the filter visualisation in the FilterGraph
    FilterResponse getResponse (double inputFrequency) const;

	// Set the filters' cutoff frequency
    void setCutoff(double cutoff);
	// Set the filters' q
//    void setQ(double Q);
private:
	// The sample rate for the filter
    double fs;

	// Reference to the main audio processor
    AudioPluginAudioProcessor& processor;
    AudioProcessorValueTreeState& vts;

	// Filter coefficients for left and right channel
    array<dsp::IIR::Coefficients<float>::Ptr, 2> coeffs;

	// Filter gain, cutoff and q
    double gainValue;
    atomic<float>* cutoff;
    atomic<float>* q;
};