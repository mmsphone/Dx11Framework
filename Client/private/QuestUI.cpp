#include "QuestUI.h"

#include "EngineUtility.h"
#include "UIImage.h"
#include "UILabel.h"
#include "Quest.h"

NS_BEGIN(Client)

QuestUI::QuestUI()
    : UI{}
{
}

QuestUI::QuestUI(const QuestUI& Prototype)
    : UI{ Prototype }
{
    m_eNpcType = Prototype.m_eNpcType;
    m_bVisible = Prototype.m_bVisible;
    m_slideState = Prototype.m_slideState;
    m_animTime = 0.f;
    m_animDuration = Prototype.m_animDuration;
    m_hiddenOffsetX = Prototype.m_hiddenOffsetX;
    m_lastQuestState = QUEST_INACTIVE;

    m_cachedBasePos = false; // 위치는 인스턴스에서 다시 캐싱
    m_baseX_backplate = 0.f;
    m_baseX_npc_unknown = 0.f;
    m_baseX_npc_female = 0.f;
    m_baseX_npc_male = 0.f;
    m_baseX_npc_unknown_text = 0.f;
    m_baseX_npc_female_text = 0.f;
    m_baseX_npc_male_text = 0.f;
    m_baseX_quest_text = 0.f;
    m_baseX_contents_title = 0.f;
    m_baseX_contents_text = 0.f;
}

HRESULT QuestUI::Initialize()
{
    UI_DESC desc{};
    desc.fRotationPerSec = 0.f;
    desc.fSpeedPerSec = 0.f;
    desc.name = "questUI";
    desc.visible = false;
    desc.enable = false;
    if (FAILED(__super::Initialize(&desc)))
        return E_FAIL;

    // 여기서는 퀘스트 UI 고유 상태만 초기화
    m_eNpcType = QUEST_NPC_UNKNOWN;
    m_bVisible = false;
    m_slideState = SLIDE_HIDDEN;
    m_animTime = 0.f;
    m_animDuration = 0.35f;
    m_hiddenOffsetX = 400.f;
    m_lastQuestState = QUEST_INACTIVE;

    m_cachedBasePos = false;

    // 처음에는 아예 숨겨둔다
    Show(false);

    return S_OK;
}

void QuestUI::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    // 퀘스트 상태 → 텍스트/애니메이션 트리거
    RefreshFromQuestManager();

    // 슬라이드 애니메이션 진행
    if (m_slideState == SLIDE_SHOWING || m_slideState == SLIDE_HIDING)
    {
        m_animTime += fTimeDelta;
        _float t = m_animTime / m_animDuration;
        if (t > 1.f) t = 1.f;

        // 간단한 ease-out
        _float ease = 1.f - (1.f - t) * (1.f - t);

        _float offset = 0.f;
        if (m_slideState == SLIDE_SHOWING)
        {
            // hiddenOffset → 0
            offset = (1.f - ease) * m_hiddenOffsetX;
        }
        else // SLIDE_HIDING
        {
            // 0 → hiddenOffset
            offset = ease * m_hiddenOffsetX;
        }

        // 위치 캐싱 + 오프셋 적용
        if (m_pEngineUtility)
        {
            if (!m_cachedBasePos)
            {
                m_cachedBasePos = true;

                auto cache = [&](const _tchar* key, _float& baseX)
                    {
                        if (UI* u = m_pEngineUtility->FindUI(key))
                        {
                            UI_DESC d = u->GetUIDesc();
                            baseX = d.x;
                        }
                    };

                cache(L"quest_backplate", m_baseX_backplate);
                cache(L"quest_npc_unknown", m_baseX_npc_unknown);
                cache(L"quest_npc_female", m_baseX_npc_female);
                cache(L"quest_npc_male", m_baseX_npc_male);
                cache(L"quest_npc_unknown_text", m_baseX_npc_unknown_text);
                cache(L"quest_npc_female_text", m_baseX_npc_female_text);
                cache(L"quest_npc_male_text", m_baseX_npc_male_text);
                cache(L"quest_quest_text", m_baseX_quest_text);
                cache(L"quest_contents_title", m_baseX_contents_title);
                cache(L"quest_contents_text", m_baseX_contents_text);
            }

            auto apply = [&](const _tchar* key, _float baseX)
                {
                    if (UI* u = m_pEngineUtility->FindUI(key))
                    {
                        UI_DESC d = u->GetUIDesc();
                        d.x = baseX + offset;
                        u->ApplyUIDesc(d);
                    }
                };

            apply(L"quest_backplate", m_baseX_backplate);
            apply(L"quest_npc_unknown", m_baseX_npc_unknown);
            apply(L"quest_npc_female", m_baseX_npc_female);
            apply(L"quest_npc_male", m_baseX_npc_male);
            apply(L"quest_npc_unknown_text", m_baseX_npc_unknown_text);
            apply(L"quest_npc_female_text", m_baseX_npc_female_text);
            apply(L"quest_npc_male_text", m_baseX_npc_male_text);
            apply(L"quest_quest_text", m_baseX_quest_text);
            apply(L"quest_contents_title", m_baseX_contents_title);
            apply(L"quest_contents_text", m_baseX_contents_text);
        }

        if (t >= 1.f)
        {
            if (m_slideState == SLIDE_SHOWING)
            {
                m_slideState = SLIDE_SHOWN;
                m_animTime = 0.f;

                // 최종 위치 정리(오프셋 0)
                if (m_pEngineUtility)
                {
                    auto apply = [&](const _tchar* key, _float baseX)
                        {
                            if (UI* u = m_pEngineUtility->FindUI(key))
                            {
                                UI_DESC d = u->GetUIDesc();
                                d.x = baseX;
                                u->ApplyUIDesc(d);
                            }
                        };

                    apply(L"quest_backplate", m_baseX_backplate);
                    apply(L"quest_npc_unknown", m_baseX_npc_unknown);
                    apply(L"quest_npc_female", m_baseX_npc_female);
                    apply(L"quest_npc_male", m_baseX_npc_male);
                    apply(L"quest_npc_unknown_text", m_baseX_npc_unknown_text);
                    apply(L"quest_npc_female_text", m_baseX_npc_female_text);
                    apply(L"quest_npc_male_text", m_baseX_npc_male_text);
                    apply(L"quest_quest_text", m_baseX_quest_text);
                    apply(L"quest_contents_title", m_baseX_contents_title);
                    apply(L"quest_contents_text", m_baseX_contents_text);
                }
            }
            else // SLIDE_HIDING
            {
                m_slideState = SLIDE_HIDDEN;
                m_animTime = 0.f;

                // 완전히 숨김
                Show(false);

                // 필요하면 위치를 hiddenOffsetX로 유지해도 되고,
                // 어차피 안 보일 거라 크게 상관은 없음.
            }
        }
    }
}

// 전체 퀘스트 UI 켜기/끄기 (즉시 온/오프)
void QuestUI::Show(_bool bShow)
{
    m_bVisible = bShow;

    // 자기 자신도 UI니까 같이 세팅
    SetVisible(bShow);

    if (!m_pEngineUtility)
        return;

    // 백플레이트 및 텍스트들
    if (UI* u = m_pEngineUtility->FindUI(L"quest_backplate"))
    {
        if (auto* img = dynamic_cast<Engine::UIImage*>(u))
        {
            img->SetBrightness(1.6f);
            img->SetGamma(0.5f);
            img->SetAlpha(0.5f);
        }
        u->SetVisible(bShow);
    }

    if (UI* u = m_pEngineUtility->FindUI(L"quest_quest_text"))
        u->SetVisible(bShow);

    if (UI* u = m_pEngineUtility->FindUI(L"quest_contents_title"))
        u->SetVisible(bShow);

    if (UI* u = m_pEngineUtility->FindUI(L"quest_contents_text"))
        u->SetVisible(bShow);

    // NPC 관련은 타입에 따라 처리
    RefreshNpcVisibility();

    if (!bShow)
    {
        // 즉시 숨김이면 슬라이드 상태도 강제로 숨김으로
        m_slideState = SLIDE_HIDDEN;
        m_animTime = 0.f;
    }
}

void QuestUI::SetNpcType(QUEST_NPC_TYPE eType)
{
    m_eNpcType = eType;
    RefreshNpcVisibility();
}

void QuestUI::RefreshNpcVisibility()
{
    if (!m_pEngineUtility)
        return;

    // 먼저 모두 끈다
    if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_unknown"))
        u->SetVisible(false);
    if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_female"))
        u->SetVisible(false);
    if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_male"))
        u->SetVisible(false);

    if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_unknown_text"))
        u->SetVisible(false);
    if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_female_text"))
        u->SetVisible(false);
    if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_male_text"))
        u->SetVisible(false);

    // 전체 UI가 꺼져 있으면 여기서 끝
    if (!m_bVisible)
        return;

    // 타입에 맞는 것만 켠다
    switch (m_eNpcType)
    {
    case QUEST_NPC_UNKNOWN:
        if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_unknown"))
            u->SetVisible(true);
        if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_unknown_text"))
            u->SetVisible(true);
        break;

    case QUEST_NPC_FEMALE:
        if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_female"))
            u->SetVisible(true);
        if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_female_text"))
            u->SetVisible(true);
        break;

    case QUEST_NPC_MALE:
        if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_male"))
            u->SetVisible(true);
        if (UI* u = m_pEngineUtility->FindUI(L"quest_npc_male_text"))
            u->SetVisible(true);
        break;

    default:
        break;
    }
}

void QuestUI::SetNpcText(const _wstring& text)
{
    if (!m_pEngineUtility)
        return;

    Engine::UILabel* pLabel = nullptr;

    switch (m_eNpcType)
    {
    case QUEST_NPC_UNKNOWN:
        pLabel = dynamic_cast<Engine::UILabel*>(
            m_pEngineUtility->FindUI(L"quest_npc_unknown_text"));
        break;
    case QUEST_NPC_FEMALE:
        pLabel = dynamic_cast<Engine::UILabel*>(
            m_pEngineUtility->FindUI(L"quest_npc_female_text"));
        break;
    case QUEST_NPC_MALE:
        pLabel = dynamic_cast<Engine::UILabel*>(
            m_pEngineUtility->FindUI(L"quest_npc_male_text"));
        break;
    default:
        break;
    }

    if (pLabel)
        pLabel->SetText(text);
}

_wstring QuestUI::WrapTextByChar(const _wstring& text, _uint maxCharsPerLine) const
{
    if (maxCharsPerLine == 0)
        return text;

    _wstring result;
    result.reserve(text.size() + text.size() / maxCharsPerLine + 8);

    _uint count = 0;

    for (wchar_t ch : text)
    {
        result.push_back(ch);

        if (ch == L'\n')
        {
            count = 0;
            continue;
        }

        ++count;

        if (count >= maxCharsPerLine)
        {
            result.push_back(L'\n');
            count = 0;
        }
    }

    return result;
}

void QuestUI::SetQuestText(const _wstring& text)
{
    if (!m_pEngineUtility)
        return;

    if (auto* label = dynamic_cast<Engine::UILabel*>(
        m_pEngineUtility->FindUI(L"quest_quest_text")))
    {
        _wstring wrapped = WrapTextByChar(text, 20); // 퀘스트 큰 텍스트는 20자 기준
        label->SetText(wrapped);
    }
}

void QuestUI::SetContentsTitle(const _wstring& text)
{
    if (!m_pEngineUtility)
        return;

    if (auto* label = dynamic_cast<Engine::UILabel*>(
        m_pEngineUtility->FindUI(L"quest_contents_title")))
    {
        label->SetText(text);
    }
}

void QuestUI::SetContentsText(const _wstring& text)
{
    if (!m_pEngineUtility)
        return;

    if (auto* label = dynamic_cast<Engine::UILabel*>(
        m_pEngineUtility->FindUI(L"quest_contents_text")))
    {
        _wstring wrapped = WrapTextByChar(text, 15); // 컨텐츠 설명은 15자 기준 줄바꿈
        label->SetText(wrapped);
    }
}

void QuestUI::RefreshFromQuestManager()
{
    if (!m_pEngineUtility)
    {
        // 엔진 유틸이 없으면 걍 숨김
        m_lastQuestState = QUEST_INACTIVE;
        if (m_slideState != SLIDE_HIDDEN)
        {
            m_slideState = SLIDE_HIDING;
            m_animTime = 0.f;
        }
        return;
    }

    _int mainId = m_pEngineUtility->GetMainQuestId();
    if (mainId < 0)
    {
        // 메인 퀘스트 없음 → 숨김
        if (m_lastQuestState == QUEST_ACTIVE ||
            m_slideState == SLIDE_SHOWN)
        {
            m_slideState = SLIDE_HIDING;
            m_animTime = 0.f;
        }
        m_lastQuestState = QUEST_INACTIVE;
        return;
    }

    Engine::Quest* pQuest = m_pEngineUtility->FindQuest(mainId);
    if (!pQuest)
    {
        if (m_lastQuestState == QUEST_ACTIVE ||
            m_slideState == SLIDE_SHOWN)
        {
            m_slideState = SLIDE_HIDING;
            m_animTime = 0.f;
        }
        m_lastQuestState = QUEST_INACTIVE;
        return;
    }

    const Engine::QUEST_DESC& q = pQuest->GetDesc();
    QUEST curState = q.state;

    // 상태 변화 → 슬라이드 트리거
    if (curState == QUEST_ACTIVE)
    {
        if (m_lastQuestState != QUEST_ACTIVE)
        {
            // 막 시작됨 → 오른쪽에서 슬라이드 인
            m_slideState = SLIDE_SHOWING;
            m_animTime = 0.f;

            // 애니메이션 동안은 보여야 하니까
            Show(true);
        }
    }
    else // INACTIVE / FAILED / COMPLETED
    {
        if (m_lastQuestState == QUEST_ACTIVE ||
            m_slideState == SLIDE_SHOWN)
        {
            // 진행 중이던 퀘스트가 끝남 → 슬라이드 아웃
            m_slideState = SLIDE_HIDING;
            m_animTime = 0.f;
        }

        m_lastQuestState = curState;
        return; // 비활성/완료 상태에서는 텍스트 갱신 안 함
    }

    m_lastQuestState = curState;

    // 여기부터는 ACTIVE 상태에서 텍스트 갱신

    // 퀘스트 큰 설명
    {
        _wstring text = q.descKey.empty() ? q.nameKey : q.descKey;
        SetQuestText(text);
    }

    // 현재 컨텐츠 텍스트 + 진행도
    if (!q.contents.empty() &&
        q.currentContentsIndex >= 0 &&
        q.currentContentsIndex < (_int)q.contents.size())
    {
        const Engine::CONTENTS_DESC& c = q.contents[q.currentContentsIndex];

        if (!c.titleKey.empty())
        {
            _wstring title = c.titleKey;

            // 필요 개수가 1보다 크면 (0/5) 같은 진행도 표시
            if (c.requiredCount > 1)
            {
                title += L" (";
                title += std::to_wstring(c.currentCount);
                title += L"/";
                title += std::to_wstring(c.requiredCount);
                title += L")";
            }
            SetContentsTitle(title);
        }

        if (!c.descKey.empty())
        {
            SetContentsText(c.descKey);
        }
    }
}

QuestUI* QuestUI::Create()
{
    QuestUI* pInstance = new QuestUI();
    if (!pInstance)
        return nullptr;

    // 프로토타입 초기화 (Base UI 쪽)
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Create : QuestUI");
        SafeRelease(pInstance);
        return nullptr;
    }

    return pInstance;
}

Object* QuestUI::Clone(void* pArg)
{
    QuestUI* pInstance = new QuestUI(*this);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Cloned : QuestUI");
        SafeRelease(pInstance);
        return nullptr;
    }

    return pInstance;
}

void QuestUI::Free()
{
    __super::Free();
}

NS_END
