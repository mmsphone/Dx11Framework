#include "Quest.h"

Quest::Quest()
    : Base{}
{
}

Quest* Quest::Create(const QUEST_DESC& desc)
{
    Quest* pInstance = new Quest;
    if (FAILED(pInstance->Initialize(desc)))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

HRESULT Quest::Initialize(const QUEST_DESC& desc)
{
    m_desc = desc;

    // 컨텐츠 상태 기본 정리
    for (auto& c : m_desc.contents)
    {
        if (c.state != CONTENTS_ACTIVE &&
            c.state != CONTENTS_COMPLETED &&
            c.state != CONTENTS_FAILED)
        {
            c.state = CONTENTS_INACTIVE;
        }

        if (c.requiredCount <= 0)
            c.requiredCount = 1;

        if (c.currentCount < 0)
            c.currentCount = 0;
    }

    // 퀘스트가 ACTIVE 상태로 들어온 경우, 첫 컨텐츠를 활성화
    if (m_desc.state == QUEST_ACTIVE && !m_desc.contents.empty())
    {
        m_desc.currentContentsIndex = 0;
        CONTENTS_DESC& first = m_desc.contents[0];
        first.state = CONTENTS_ACTIVE;
        first.currentCount = 0;

        if (m_desc.onStart)
            m_desc.onStart();
        if (first.onStart)
            first.onStart();
    }
    else
    {
        // 그 외에는 일단 INACTIVE로 통일
        if (m_desc.state != QUEST_COMPLETED &&
            m_desc.state != QUEST_FAILED)
        {
            m_desc.state = QUEST_INACTIVE;
        }
        m_desc.currentContentsIndex = 0;
    }

    return S_OK;
}

void Quest::Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    // 시간 기반 컨텐츠(타이머 등)가 필요해지면 여기서 처리
}

void Quest::OnEvent(const QUEST_EVENT& ev)
{
    if (m_desc.state != QUEST_ACTIVE)
        return;

    if (m_desc.currentContentsIndex < 0 ||
        m_desc.currentContentsIndex >= (_int)m_desc.contents.size())
        return;

    CONTENTS_DESC& cur = m_desc.contents[m_desc.currentContentsIndex];

    if (cur.state != CONTENTS_ACTIVE)
        return;

    ProcessContentsEvent(cur, ev);

    if (cur.state == CONTENTS_COMPLETED)
    {
        if (cur.onCompleted)
            cur.onCompleted();
        TryAdvanceContents();
    }
}

QUEST_DESC& Quest::GetDesc()
{
    return m_desc;
}

QUEST Quest::GetState() const
{
    return m_desc.state;
}

_int Quest::GetId() const
{
    return m_desc.id;
}

void Quest::SetOwner(QuestManager* owner)
{
    m_pOwner = owner;
}

void Quest::ProcessContentsEvent(CONTENTS_DESC& contents, const QUEST_EVENT& ev)
{
    if (contents.state != CONTENTS_ACTIVE)
        return;

    auto MatchTag = [&]() -> bool
        {
            if (contents.targetTag.empty())
                return true;
            return contents.targetTag == ev.tag;
        };

    auto MatchId = [&]() -> bool
        {
            if (contents.triggerId < 0)
                return true;
            return contents.triggerId == ev.intParam;
        };

    bool progressed = false;

    switch (contents.type)
    {
    case CONTENTSTYPE_TRIGGER:
        // 트리거 인/아웃 중 뭘 쓸지는 규칙에 따라 변경 가능
        if (ev.type == EVENTTYPE_TRIGGER_IN && MatchTag() && MatchId())
        {
            contents.currentCount++;
            progressed = true;
        }
        break;

    case CONTENTSTYPE_KILL:
        if (ev.type == EVENTTYPE_KILL && MatchTag())
        {
            contents.currentCount++;
            progressed = true;
        }
        break;

    case CONTENTSTYPE_INTERACT:
        if (ev.type == EVENTTYPE_INTERACT && MatchTag())
        {
            contents.currentCount++;
            progressed = true;
        }
        break;

    case CONTENTSTYPE_CUSTOM:
        // 완전 커스텀이지만, 기본 구현은 태그/ID 맞으면 1 증가
        if (ev.type == EVENTTYPE_CUSTOM && MatchTag() && MatchId())
        {
            contents.currentCount++;
            progressed = true;
        }
        break;

    default:
        break;
    }

    if (!progressed)
        return;

    if (contents.requiredCount <= 0)
        contents.requiredCount = 1;

    if (contents.currentCount >= contents.requiredCount)
    {
        contents.currentCount = contents.requiredCount;
        contents.state = CONTENTS_COMPLETED;
    }
}

void Quest::TryAdvanceContents()
{
    if (m_desc.state != QUEST_ACTIVE)
        return;

    if (m_desc.currentContentsIndex < 0 ||
        m_desc.currentContentsIndex >= (_int)m_desc.contents.size())
    {
        // 더 이상 진행할 컨텐츠가 없으니 퀘스트 완료 처리
        m_desc.state = QUEST_COMPLETED;

        if (m_desc.onCompleted)
            m_desc.onCompleted();

        return;
    }

    _int nextIndex = m_desc.currentContentsIndex + 1;

    if (nextIndex >= (_int)m_desc.contents.size())
    {
        // 마지막 컨텐츠까지 완료 → 퀘스트 완료
        m_desc.state = QUEST_COMPLETED;

        if (m_desc.onCompleted)
            m_desc.onCompleted();

        return;
    }

    m_desc.currentContentsIndex = nextIndex;

    CONTENTS_DESC& next = m_desc.contents[nextIndex];

    // 새 컨텐츠 활성화
    if (next.state != CONTENTS_COMPLETED &&
        next.state != CONTENTS_FAILED)
    {
        next.state = CONTENTS_ACTIVE;
        if (next.requiredCount <= 0)
            next.requiredCount = 1;
        if (next.currentCount < 0)
            next.currentCount = 0;

        if (next.onStart)
            next.onStart();
    }
}

void Quest::Free()
{
    m_desc.contents.clear();
    __super::Free();
}
