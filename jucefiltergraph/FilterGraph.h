/*
  ==============================================================================

    FilterGraph.h
    
    Adapted from Sean Enderby's implementation: https://sourceforge.net/projects/jucefiltergraph/

  ==============================================================================
*/
#pragma once
#ifndef FILTER_GRAPH_H
#define FILTER_GRAPH_H

#include "FilterInfo.h"


using namespace std;

//==============================================================================
class FilterGraph    : public Component,
                       public SettableTooltipClient,
    private AudioProcessorValueTreeState::Listener, ChangeBroadcaster, ChangeListener
{
public:
    FilterGraph(array<dsp::IIR::Filter<float>, 2>& lowpassFilters,
            array<dsp::IIR::Filter<float>, 2>& highpassFilters,
            double sampleRate,
            AudioProcessorValueTreeState&,
            TooltipWindow&);
    ~FilterGraph();
    
    enum TraceType
    {
        Magnitude,
        Phase
    };
    
    void paint (Graphics&) override;
    void resized() override;

	// Convert from point on component to frequency
    float xToFreq (float xPos) const;
	// Convert frequency to point on component
    float freqToX (float freq) const;

    void setTraceColour (Colour newColour);
    
    float maxdB, maxPhas;
    Colour traceColour;

	// Can be configured to plot either filter's response to magnitude or phase
    TraceType traceType;
    
private:
	// Reference to TooltipWindow instance
	// Necessary to display tooltips
    TooltipWindow& tooltip;

	// Number of horizontal lines to display
    int numHorizontalLines;

	// Low and high frequency limits
    float lowFreq, highFreq;
	
    // Sample rate
	double fs;
	// Number of filters
    int numFilters;

    // Flag to check if user is dragging mouse over component
    bool isDragging = false;
	// Placeholders for last mouse positions
	// Relative to component
    Point<int> lastMousePosRel;
	// Relative to screen
    Point<int> lastMousePosAbs;
    void mouseMove (const MouseEvent &event) override;
    void mouseDrag (const MouseEvent& event) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;

	// Update the filter values in state management and in the FilterInfo
    void updateFilter(int filter, float cutoff);

	// Render a tooltip showing the current cutoff frequency and q value
	// nn drag
    void renderTooltip(Point<int>& mousePosRel, Point<int>& mousePosAbs);

	// Reference to state management tree
    AudioProcessorValueTreeState& vts;
    Value lowpassCutoff;
    Value highpassCutoff;
    atomic<float>* numberOfBands;

    int selectedFilterDragging = 0;

	// Vector containing the displayed filters
    vector <FilterInfo> filterVector;

	// Paths for the grid display and trace of filter response
    Path gridPath, tracePath;

	// Callback methods invoked on parameter / UI change
    void parameterChanged(const String& parameterID, float newValue) override;
    void changeListenerCallback(ChangeBroadcaster* source) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterGraph)
};

#endif