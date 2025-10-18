#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL Panel : public Base
{
protected:
    Panel(const string& PanelName, bool open = true);
    virtual ~Panel() = default;

public:
    void Draw();
    virtual void OnRender() = 0;
    const string& GetPanelName() const;

    void SetPanelPos(_float2& vPos);
    void SetPanelSize(_float2& vSize);
    void SetOpen(bool value);
    bool IsOpen() const;

    virtual void		Free() override;
protected:
    class EngineUtility* m_pEngineUtility = { nullptr };
    const string m_PanelName;
    bool m_IsOpen;
    _float2 m_PanelPosition = { 50.f, 50.f };
    _float2 m_PanelSize = { 400.f, 600.f };
};

NS_END