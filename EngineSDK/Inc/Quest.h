#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class QuestManager;

class ENGINE_DLL Quest : public Base
{
	Quest();
	virtual ~Quest() = default;

public:
    static Quest* Create(const QUEST_DESC& desc);

    HRESULT Initialize(const QUEST_DESC& desc);
    void    Update(_float fTimeDelta);
    void    OnEvent(const QUEST_EVENT& ev);

    QUEST_DESC& GetDesc();

    QUEST  GetState() const;
    _int   GetId()    const;

    void   SetOwner(QuestManager* owner);

    virtual void Free() override;

private:
    void    ProcessContentsEvent(CONTENTS_DESC& contents, const QUEST_EVENT& ev);
    void    TryAdvanceContents();

private:
    QUEST_DESC m_desc;
    QuestManager* m_pOwner = { nullptr };
};

NS_END