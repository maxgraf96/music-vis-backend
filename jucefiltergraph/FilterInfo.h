/*
  ==============================================================================

    FilterInfo.h
    
    Wrapper for filters and their responses.
	Adapted from Sean Enderby's implementation: https://sourceforge.net/projects/jucefiltergraph/

  ==============================================================================
*/
#ifndef FILTER_INFO_H
#define FILTER_INFO_H

#include <juce_dsp/juce_dsp.h>
#include "../PluginProcessor.h"
#include "../Constants.h"
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
    enum FilterType {
        LOWPASS = 0,
        HIGHPASS
    };

    FilterInfo(array<dsp::IIR::Filter<float>, 2>& filters, FilterType type, double sampleRate, AudioProcessorValueTreeState& valueTreeState);
    ~FilterInfo();
    
    void setSampleRate (double sampleRate);
    void setGain (double gain);

	// This method was adapted for modern JUCE versions
	// It calls the getMagnitudeForFrequency(...) and getPhaseForFrequency(...)
	// methods to generate the paths used to draw the filter visualisation in the FilterGraph
    FilterResponse getResponse (double inputFrequency) const;

    // Get the filter type (lowpass/highpass)
    FilterType getFilterType();

private:
    // Filter type: 0 = lowpass, 1 = highpass
    FilterType filterType = LOWPASS;

	// The sample rate for the filter
    double fs;

	// Reference to the value tree state from the processor to pick up state changes
    AudioProcessorValueTreeState& vts;

	// References to filters
    array<dsp::IIR::Filter<float>, 2>& filters;

	// Filter gain, cutoff and q
    double gainValue;
};

#endif