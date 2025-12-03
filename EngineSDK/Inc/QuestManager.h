// QuestManager.h
#pragma once

#include "Quest.h"

NS_BEGIN(Engine)

class ENGINE_DLL QuestManager final : public Base
{
private:
    QuestManager();
    virtual ~QuestManager() = default;

public:
    HRESULT Initialize();
    void    Update(_float fTimeDelta);

    Quest* AddQuest(const QUEST_DESC& desc);
    Quest* FindQuest(_int questId);
    void ClearQuest();

    void         StartQuest(_int questId);
    QUEST        GetQuestState(_int questId);

    void         PushEvent(const QUEST_EVENT& ev);

    void         SetMainQuestId(_int questId);
    _int         GetMainQuestId() const;

    void         SetQuestStartFunction(_int questId, const std::function<void()>& func);
    void         SetQuestCompleteFunction(_int questId, const std::function<void()>& func);
    void         SetContentsStartFunction(_int questId, _int contentsIndex, const std::function<void()>& func);
    void         SetContentsCompleteFunction(_int questId, _int contentsIndex, const std::function<void()>& func);

    static QuestManager* Create();
    virtual void Free() override;
private:
    void         ProcessEvents();

private:
    std::vector<Quest*>     m_Quests;
    std::vector<QUEST_EVENT> m_EventQueue;

    _int m_mainQuestId = -1;
};

NS_END
