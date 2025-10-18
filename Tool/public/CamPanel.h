#pragma once

#include "Tool_Defines.h"
#include "Panel.h"

namespace Engine
{
    class Object;
}

NS_BEGIN(Tool)

class CamPanel final : public Panel
{
    CamPanel(const string& PanelName, bool open = true);
    virtual ~CamPanel() = default;

public:
    HRESULT Initialize(SCENE eType);
    virtual void OnRender() override;

    static CamPanel* Create(const string& PanelName, SCENE eType, bool open = true);
    virtual void		Free() override;

private:
    Object* m_pCamObject = nullptr;
    SCENE m_SceneType = SCENE_END;
};

NS_END