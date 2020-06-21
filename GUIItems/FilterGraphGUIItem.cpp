//
// Created by Max on 21/06/2020.
//

#include "FilterGraphGUIItem.h"
#include "../PluginProcessor.h"

FilterGraphGUIItem::FilterGraphGUIItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
    :foleys::GuiItem (builder, node){
    if (auto* proc = dynamic_cast<AudioPluginAudioProcessor*>(builder.getMagicState().getProcessor()))
    {
        filterGraph = make_unique<FilterGraph>(proc->getLowpassFilters(),
                proc->getHighpassFilters(),
                proc->getSampleRate(),
                proc->getMagicState().getValueTreeState(),
                proc->getTooltipWindow());
        addAndMakeVisible (filterGraph.get());
    }
}

void FilterGraphGUIItem::update() {

}
