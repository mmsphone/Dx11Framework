#pragma once
#include "Panel.h"

#include "Tool_Defines.h"

namespace Engine
{
    class Object;
}

NS_BEGIN(Tool)

class ObjectPanel : public Panel
{
    ObjectPanel(const string& PanelName, bool open = true);
    virtual ~ObjectPanel() = default;

public:
    HRESULT Initialize(Object* pTarget);
    virtual void OnRender() override;

    static ObjectPanel* Create(const string& PanelName, bool open = true, Object* pTarget = nullptr);
    virtual void		Free() override;

private:
    Object* m_pTargetObject = nullptr;
};

NS_END