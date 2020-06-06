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
FilterInfo::FilterInfo(AudioPluginAudioProcessor& p, AudioProcessorValueTreeState& valueTreeState)
    :processor(p), vts(valueTreeState)
{
    // Get the filters' coeffitiens (currently they are the same for both filters)
//    this->coeffs[0] = p.getFilters()[0].coefficients;
//    this->coeffs[1] = p.getFilters()[1].coefficients;

    // Get initial values from VTS
    this->cutoff = vts.getRawParameterValue("mainFilterCutoff");
    this->q = vts.getRawParameterValue("mainFilterQ");

    fs = p.getSampleRate();
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

void FilterInfo::updateFilter(float cutoff, float q)
{
	// Update filter coefficients
    for (auto& coefs : coeffs)
        coefs = coefs->makeLowPass(fs, cutoff, q);
}

FilterResponse FilterInfo::getResponse (double inputFrequency) const
{
	// This 
//	const double mag = coeffs[0]->getMagnitudeForFrequency(inputFrequency, fs);
//	const double phase = coeffs[0]->getPhaseForFrequency(inputFrequency, fs);

	// Wrap in FilterResponse
//    return FilterResponse(mag, phase);
    return FilterResponse(0, 0);
}

void FilterInfo::setCutoff(double cutoff)
{
    *this->cutoff = cutoff;
    updateFilter(cutoff, *q);
}

//void FilterInfo::setQ(double Q)
//{
//    this->q = Q;
//    updateFilter(cutoff, q);
//}
