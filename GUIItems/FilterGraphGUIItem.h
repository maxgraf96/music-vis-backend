//
// Created by Max on 21/06/2020.
//
#pragma once
#ifndef MUSIC_VIS_BACKEND_FILTERGRAPHGUIITEM_H
#define MUSIC_VIS_BACKEND_FILTERGRAPHGUIITEM_H

#include "../foleys_gui_magic/foleys_gui_magic.h"
#include "../jucefiltergraph/FilterGraph.h"

class FilterGraphGUIItem : public foleys::GuiItem {
public:
    FOLEYS_DECLARE_GUI_FACTORY (FilterGraphGUIItem)

    FilterGraphGUIItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node);

    void update() override;

    juce::Component* getWrappedComponent() override
    {
        return filterGraph.get();
    }

private:
    unique_ptr<FilterGraph> filterGraph;
};


#endif //MUSIC_VIS_BACKEND_FILTERGRAPHGUIITEM_H
