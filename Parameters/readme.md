The parameter classes here are merely wrapper classes for normal JUCE audio parameters.
AU plugin validation requires parameters that can be changed by other parameters to be marked as
"meta". Since there is no native way to do that with traditional parameters in JUCE yet, 
these wrapper classes were created.