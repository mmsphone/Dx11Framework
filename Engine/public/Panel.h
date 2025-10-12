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

    void SetOpen(bool value);
    bool IsOpen() const;

    virtual void		Free() override;
protected:
    class EngineUtility* m_pEngineUtility = { nullptr };
    const string m_PanelName;
    bool m_IsOpen;
};

NS_END