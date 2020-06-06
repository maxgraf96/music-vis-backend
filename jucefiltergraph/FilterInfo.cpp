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
    // Get initial values from VTS
    this->cutoff = vts.getRawParameterValue("mainFilterCutoff");
    this->q = vts.getRawParameterValue("mainFilterQ");

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

void FilterInfo::updateFilter(float cutoff, float q)
{
	// Update filter coefficients
    for (auto & filter : filters) {
        if(filterType == LOWPASS){
            filter.coefficients = dsp::IIR::Coefficients<float>::makeLowPass(fs, cutoff, q);
        } else if (filterType == HIGHPASS){
            filter.coefficients = dsp::IIR::Coefficients<float>::makeHighPass(fs, cutoff, q);
        }
    }
}

FilterResponse FilterInfo::getResponse (double inputFrequency) const
{
	const double mag = filters[0].coefficients.get()->getMagnitudeForFrequency(inputFrequency, fs);
	const double phase = filters[0].coefficients.get()->getPhaseForFrequency(inputFrequency, fs);

	// Wrap in FilterResponse
    return FilterResponse(mag, phase);
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
