#pragma once

#include "Tool_Defines.h"
#include "Panel.h"

namespace Engine 
{
    class Object;
}

NS_BEGIN(Tool)

class ModelPanel final : public Panel
{
    ModelPanel(const string& PanelName, bool open = true);
    virtual ~ModelPanel() = default;

public:
    virtual void OnRender() override;

    static ModelPanel* Create(const string& PanelName, bool open = true);
    virtual void		Free() override;

private:
    Object* m_pModelObject = nullptr;
};

NS_END