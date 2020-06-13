/*
  ==============================================================================

    FilterGraph.cpp
    
    Adapted from Sean Enderby's implementation: https://sourceforge.net/projects/jucefiltergraph/

  ==============================================================================
*/

#include "FilterGraph.h"

FilterGraph::FilterGraph(AudioPluginAudioProcessor& p, AudioProcessorValueTreeState& valueTreeState, TooltipWindow& tooltip)
    :tooltip(tooltip), vts(valueTreeState)
{
    // Construct filter vector from low- and highpass filters from processor
    filterVector.emplace_back(FilterInfo(p.getLowpassFilters(), FilterInfo::LOWPASS, p.getSampleRate(), valueTreeState));
    filterVector.emplace_back(FilterInfo(p.getHighpassFilters(), FilterInfo::HIGHPASS, p.getSampleRate(), valueTreeState));

    numHorizontalLines = 7;
	// Hard limit frequency region for now
    lowFreq = 20;
    highFreq = 20000;
    fs = p.getSampleRate();
    maxdB = 6;
    maxPhas = 1;
    numFilters = 2;
    traceColour = Colour(0xaa00ff00);
    traceType = Magnitude;

	// Add parameter listeners
    vts.addParameterListener("lowpassCutoff", this);
    vts.addParameterListener("highpassCutoff", this);
    vts.addParameterListener("numberOfBands", this);

    // Update values
    lowpassCutoff.referTo(vts.getParameterAsValue("lowpassCutoff"));
    float lowpassCutoffVal = *vts.getRawParameterValue("lowpassCutoff");
    lowpassCutoff = lowpassCutoffVal;

    highpassCutoff.referTo(vts.getParameterAsValue("highpassCutoff"));
    float highpassCutoffVal = *vts.getRawParameterValue("highpassCutoff");
    highpassCutoff = highpassCutoffVal;

    numberOfBands = vts.getRawParameterValue("numberOfBands");

    repaint();

	// Add change listener in order to pickup UI/audioparameter changes
	// and be able to repaint asynchronously
    addChangeListener(this);
}

FilterGraph::~FilterGraph()
{
	// Remove state management listeners
    vts.removeParameterListener("lowpassCutoff", this);
    vts.removeParameterListener("highpassCutoff", this);
}

void FilterGraph::paint (Graphics& g)
{
    // get size info =======================================================================================
    const auto width = float(getWidth());
    const auto height = float(getHeight());
    
    // paint the display background
    g.setGradientFill (ColourGradient (Colour (0xff232338), width / 2, height / 2, Colour (0xff21222a), 2.5f, height / 2, true));
    g.fillRect (2.5f, 2.5f, width - 5, height - 5);

    for (FilterInfo& filterInfo : filterVector){
        // Clear trace path
        tracePath.clear();

        if (traceType == Magnitude)
        {
            const float scaleFactor = (((height / 2) - (height - 5) / (numHorizontalLines + 1) - 2.5f) / maxdB);

            float traceMagnitude = float(filterInfo.getResponse(lowFreq).magnitudeValue);
            traceMagnitude = 20 * log10 (traceMagnitude);

            tracePath.startNewSubPath (2.5f, (height / 2) - (traceMagnitude * scaleFactor));

            for (float xPos = 3.5; xPos < (width - 2.5); xPos += 1.0f)
            {
                const float freq = xToFreq (xPos);

                traceMagnitude = float(filterInfo.getResponse(freq).magnitudeValue);
                // Convert to dB
                traceMagnitude = 20 * log10 (traceMagnitude);
                // Trace path
                tracePath.lineTo (xPos, (height / 2) - (traceMagnitude * scaleFactor));
            }
        }

        // DRAW
        if(filterInfo.getFilterType() == FilterInfo::FilterType::LOWPASS){
            g.setColour (traceColour);
        } else {
            g.setColour(traceColour.contrasting());
        }
        g.strokePath (tracePath, PathStrokeType (3.0f));
    }
    
    // paint the display grid lines ===============================================================================
    g.setColour (Colour (0xaaffffff));
    String axisLabel;
    if (traceType == Magnitude)  axisLabel = String (maxdB, 1) + "dB";
    else if (traceType == Phase) 
    {
        axisLabel = String (CharPointer_UTF8 ("\xcf\x80")) + "rad";
        axisLabel = (maxPhas == 1) ? axisLabel : String (maxPhas, 1) + axisLabel;
    }
    
    g.setFont (Font ("Open Sans", 12.0f, Font::plain));
    g.drawText (axisLabel, 6, (int) ((height - 5) / (numHorizontalLines + 1) -9.5f), 45, 12, Justification::left, false);
    g.drawText (String ("-") + axisLabel, 6, (int) (numHorizontalLines * (height - 5) / (numHorizontalLines + 1) + 3.5f), 45, 12, Justification::left, false);
    
    gridPath.clear();
    for (int lineNum = 1; lineNum < numHorizontalLines + 1; lineNum++)
    {
        float yPos = lineNum * (height - 5) / (numHorizontalLines + 1) + 2.5f;
        gridPath.startNewSubPath (2.5f, yPos);
        gridPath.lineTo (width - 2.5f, yPos);
    }
    
    float order = (pow (10, floor (log10 (lowFreq))));
    float rounded = order * (floor(lowFreq / order) + 1);
    for (float freq = rounded; freq < highFreq; freq += pow (10, floor (log10 (freq))))
    {
        float xPos = freqToX (freq);
        gridPath.startNewSubPath (xPos, 2.5f);
        gridPath.lineTo (xPos, height - 2.5f);
    }
    g.excludeClipRegion (Rectangle <int> (6, (int) ((height - 5) / (numHorizontalLines + 1) -9.5f), 45, 12));
    g.excludeClipRegion (Rectangle <int> (6, (int) (numHorizontalLines * (height - 5) / (numHorizontalLines + 1) + 3.5f), 45, 12));
    g.setColour (Colour (0x60ffffff));   
    g.strokePath (gridPath, PathStrokeType (1.0f));
    
    // draw the border ======================================================================================
    g.setColour (Colours::black);
    g.drawRect (2.5f, 2.5f, width - 5, height - 5, 1.000f);

    // Draw tooltip if dragging
    if (isDragging) {
        renderTooltip(lastMousePosRel, lastMousePosAbs);
    }
}

void FilterGraph::resized()
{
}

float FilterGraph::xToFreq (float xPos) const
{
	const auto width = float(getWidth());
    return lowFreq * pow ((highFreq / lowFreq), ((xPos - 2.5f) / (width - 5.0f)));
}

float FilterGraph::freqToX (float freq) const
{
	const auto width = float(getWidth());
    return (width - 5) * (log (freq / lowFreq) / log (highFreq / lowFreq)) + 2.5f;
}

void FilterGraph::setTraceColour (Colour newColour)
{
    traceColour = newColour;
    repaint();
}

void FilterGraph::mouseMove (const MouseEvent &event)
{    
    repaint();
}

void FilterGraph::mouseDrag(const MouseEvent& event)
{
    lastMousePosRel = getMouseXYRelative();
    lastMousePosAbs = event.getScreenPosition();
    
    // Get x position for cutoff frequency
    int xPos = lastMousePosRel.getX();
    if (xPos < 0) xPos = 0;
    if (xPos > getWidth()) xPos = getWidth();

    // Convert from coordinates to values
    const float freq = xToFreq(xPos);

    // Determine which filter's cutoff frequency should be moved
    // 2 bands (value is off by one due to JUCE combobox handling)
    if(*numberOfBands == 1.0f){
        // Update both bands
        updateFilter(0, freq);
        updateFilter(1, freq);
    }
    // 3 bands
    else if (*numberOfBands == 2.0f){
        // Update filter coefficients
        updateFilter(selectedFilterDragging, freq);
    }

	// Asynchronous repaint
    sendChangeMessage();
}

void FilterGraph::mouseDown(const MouseEvent& event) {
    isDragging = true;

    const float freq = xToFreq(getMouseXYRelative().getX());
    if(abs(freq - static_cast<float>(lowpassCutoff.getValue())) < abs(freq - static_cast<float>(highpassCutoff.getValue()))){
        selectedFilterDragging = 0;
    } else {
        selectedFilterDragging = 1;
    }
}

void FilterGraph::mouseUp(const MouseEvent& event) {
    isDragging = false;
}

void FilterGraph::updateFilter(int filter, float cutoff) {
    if (filter == 0){
        lowpassCutoff = cutoff;
    } else {
        highpassCutoff = cutoff;
    }
}

void FilterGraph::renderTooltip(Point<int>& mousePosRel, Point<int>& mousePosAbs) {
    int xPos = lastMousePosRel.getX();
    if (xPos < 0)
        xPos = 0;
    if (xPos > getWidth())
        xPos = getWidth();

	// Convert mouse position to frequency
    const float freq = xToFreq(xPos);

    // Display tooltip
    if (traceType == Magnitude)
    {
    	// Get magnitude for current mouse position
        float magnitude = float(filterVector[0].getResponse(freq).magnitudeValue);
    	// Convert to dB
        magnitude = 20 * log10(magnitude);
        // Display tooltip
        tooltip.displayTip(mousePosAbs, String(freq, 1) + "Hz, " + String(magnitude, 1) + "dB");
    }
}

void FilterGraph::parameterChanged(const String& parameterID, float newValue)
{
    if (parameterID == "lowpassCutoff") {
    	// Repaint asynchronously
        sendChangeMessage();
    }
    if (parameterID == "highpassCutoff") {
        // Repaint asynchronously
        sendChangeMessage();
    }
}

void FilterGraph::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == this) 
        repaint();
}
