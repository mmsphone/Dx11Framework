#pragma once

#include "Tool_Defines.h"
#include "Panel.h"

NS_BEGIN(Tool)

class MapPanel : public Panel
{
    MapPanel(const string& PanelName, bool open = true);
    virtual ~MapPanel() = default;

public:
    virtual void OnRender() override;

    static MapPanel* Create(const string& PanelName, bool open = true);
    virtual void		Free() override;
};

NS_END