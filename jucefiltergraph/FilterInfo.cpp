/*
  ==============================================================================

    FilterInfo.cpp
    
    Sean Enderby

  ==============================================================================
*/

#include "FilterInfo.h"

//==============================================================================
FilterResponse::FilterResponse (double magnitudeInit, double phaseInit)
{
    magnitudeValue = magnitudeInit;
    phaseValue = phaseInit;
}

FilterResponse::~FilterResponse()
{
}

//===============================================================================
FilterInfo::FilterInfo(array<dsp::IIR::Filter<float>, 2>& filters, FilterType type, double sampleRate, AudioProcessorValueTreeState& valueTreeState)
    : vts(valueTreeState), filters(filters), filterType(type)
{
    fs = sampleRate;
    gainValue = 1;
}

FilterInfo::~FilterInfo()
{
}

void FilterInfo::setSampleRate (double sampleRate)
{
    fs = sampleRate;
}

void FilterInfo::setGain (double gain)
{
    gainValue = gain;
}

FilterResponse FilterInfo::getResponse (double inputFrequency) const
{
	const double mag = filters[0].coefficients.get()->getMagnitudeForFrequency(inputFrequency, fs);
	const double phase = filters[0].coefficients.get()->getPhaseForFrequency(inputFrequency, fs);

	// Wrap in FilterResponse
    return FilterResponse(mag, phase);
}

FilterInfo::FilterType FilterInfo::getFilterType() {
    return filterType;
}