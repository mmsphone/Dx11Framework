#include "QuestManager.h"

NS_BEGIN(Engine)

QuestManager::QuestManager()
    : Base{}
    , m_mainQuestId{ -1 }
{
}

HRESULT QuestManager::Initialize()
{
    m_Quests.clear();
    m_EventQueue.clear();
    m_mainQuestId = -1;
    return S_OK;
}

void QuestManager::Update(_float fTimeDelta)
{
    ProcessEvents();

    for (Quest* pQuest : m_Quests)
    {
        if (pQuest)
            pQuest->Update(fTimeDelta);
    }
}

Quest* QuestManager::AddQuest(const QUEST_DESC& desc)
{
    Quest* pQuest = Quest::Create(desc);
    if (!pQuest)
        return nullptr;

    pQuest->SetOwner(this);

    m_Quests.push_back(pQuest);
    return pQuest;
}

Quest* QuestManager::FindQuest(_int questId)
{
    for (Quest* pQuest : m_Quests)
    {
        if (pQuest && pQuest->GetId() == questId)
            return pQuest;
    }
    return nullptr;
}

void QuestManager::ClearQuest()
{
    for (Quest* pQuest : m_Quests)
        SafeRelease(pQuest);
    m_Quests.clear();
}

void QuestManager::StartQuest(_int questId)
{
    Quest* pQuest = FindQuest(questId);
    if (!pQuest)
        return;

    QUEST_DESC& desc = pQuest->GetDesc();

    // 퀘스트 상태 활성화
    desc.state = QUEST_ACTIVE;

    // 컨텐츠 상태 초기화 후 첫 컨텐츠 활성화
    if (!desc.contents.empty())
    {
        for (auto& c : desc.contents)
        {
            c.state = CONTENTS_INACTIVE;
            c.currentCount = 0;
            if (c.requiredCount <= 0)
                c.requiredCount = 1;
        }

        desc.currentContentsIndex = 0;
        CONTENTS_DESC& first = desc.contents[0];
        first.state = CONTENTS_ACTIVE;

        // ★ 첫 컨텐츠 시작 콜백
        if (first.onStart)
            first.onStart();
    }
    if (desc.onStart)
        desc.onStart();
}

QUEST QuestManager::GetQuestState(_int questId)
{
    Quest* pQuest = FindQuest(questId);
    if (!pQuest)
        return QUEST_INACTIVE;

    return pQuest->GetState();
}

void QuestManager::PushEvent(const QUEST_EVENT& ev)
{
    m_EventQueue.push_back(ev);
}

void QuestManager::SetMainQuestId(_int questId)
{
    m_mainQuestId = questId;
}

_int QuestManager::GetMainQuestId() const
{
    return m_mainQuestId;
}

void QuestManager::SetQuestStartFunction(_int questId, const std::function<void()>& func)
{
    Quest* pQuest = FindQuest(questId);
    if (!pQuest) return;

    pQuest->GetDesc().onStart = func;
}

void QuestManager::SetQuestCompleteFunction(_int questId, const std::function<void()>& func)
{
    Quest* pQuest = FindQuest(questId);
    if (!pQuest) return;

    pQuest->GetDesc().onCompleted = func;
}

void QuestManager::SetContentsStartFunction(_int questId, _int contentsIndex, const std::function<void()>& func)
{
    Quest* pQuest = FindQuest(questId);
    if (!pQuest) return;

    QUEST_DESC& desc = pQuest->GetDesc();
    if (contentsIndex < 0 || contentsIndex >= (_int)desc.contents.size())
        return;

    desc.contents[contentsIndex].onStart = func;
}

void QuestManager::SetContentsCompleteFunction(_int questId, _int contentsIndex, const std::function<void()>& func)
{
    Quest* pQuest = FindQuest(questId);
    if (!pQuest) return;

    QUEST_DESC& desc = pQuest->GetDesc();
    if (contentsIndex < 0 || contentsIndex >= (_int)desc.contents.size())
        return;

    desc.contents[contentsIndex].onCompleted = func;
}

void QuestManager::ProcessEvents()
{
    if (m_EventQueue.empty())
        return;

    for (const QUEST_EVENT& ev : m_EventQueue)
    {
        for (Quest* pQuest : m_Quests)
        {
            if (pQuest)
                pQuest->OnEvent(ev);
        }
    }

    m_EventQueue.clear();
}

QuestManager* QuestManager::Create()
{
    QuestManager* pInstance = new QuestManager;
    if (FAILED(pInstance->Initialize()))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void QuestManager::Free()
{
    for (Quest* pQuest : m_Quests)
    {
        SafeRelease(pQuest);
    }
    m_Quests.clear();
    m_EventQueue.clear();

    __super::Free();
}

NS_END
