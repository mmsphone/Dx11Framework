#pragma once

#include "Tool_Defines.h"
#include "Panel.h"

NS_BEGIN(Tool)

class ModelPanel : public Panel
{
    ModelPanel(const string& PanelName, bool open = true);
    virtual ~ModelPanel() = default;

public:
    virtual void OnRender() override;

    static ModelPanel* Create(const string& PanelName, bool open = true);
    virtual void		Free() override;
};

NS_END