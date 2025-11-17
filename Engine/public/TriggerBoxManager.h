#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class TriggerBoxManager final : public Base
{
    TriggerBoxManager();
    virtual ~TriggerBoxManager() = default;

public:
    HRESULT Initialize();

    HRESULT AddTriggerBox(class TriggerBox* pTriggerBox);
    HRESULT RemoveTriggerBox(_uint index);
    void ClearTriggerBoxes();

    void UpdateTriggers();
    void RenderTriggerBoxes();
    const vector<class TriggerBox*>& GetTriggerBoxes() const;

    static TriggerBoxManager* Create();
    virtual void              Free() override;

private:
    vector<class TriggerBox*> m_TriggerBoxes;
};

NS_END