#pragma once

#include "Client_Defines.h"
#include "UI.h"

NS_BEGIN(Client)

enum QUEST_NPC_TYPE : _int
{
    QUEST_NPC_UNKNOWN = 0,
    QUEST_NPC_FEMALE,
    QUEST_NPC_MALE,
};

class QuestUI : public UI
{
    QuestUI();
    QuestUI(const QuestUI& Prototype);
    virtual ~QuestUI() = default;

public:
    HRESULT Initialize();
    void    Update(_float fTimeDelta);

    // 전체 퀘스트 UI 켜기/끄기
    void    Show(_bool bShow);

    // NPC 초상/이름 타입 (unknown/female/male)
    void    SetNpcType(QUEST_NPC_TYPE eType);

    // 현재 NPC에 표시될 텍스트(이름/대사 등)
    void    SetNpcText(const _wstring& text);

    // 퀘스트/컨텐츠 텍스트 직접 세팅
    void    SetQuestText(const _wstring& text);
    void    SetContentsTitle(const _wstring& text);
    void    SetContentsText(const _wstring& text);

    // QuestManager 상태로부터 자동으로 UI 갱신
    void    RefreshFromQuestManager();

    static QuestUI* Create();
    Object* Clone(void* pArg) override;
    virtual void Free() override;

private:
    void    RefreshNpcVisibility(); // m_eNpcType, m_bVisible 기반으로 NPC UI On/Off
    _wstring WrapTextByChar(const _wstring& text, _uint maxCharsPerLine) const;

    enum SLIDE_STATE : int
    {
        SLIDE_HIDDEN,
        SLIDE_SHOWING,
        SLIDE_SHOWN,
        SLIDE_HIDING
    };

private:
    QUEST_NPC_TYPE m_eNpcType = QUEST_NPC_UNKNOWN;
    _bool          m_bVisible = false;

    //퀘스트창 슬라이드
    SLIDE_STATE m_slideState = SLIDE_HIDDEN;
    _float            m_animTime = 0.f;
    _float            m_animDuration = 0.5f;   // 슬라이드 시간
    _float            m_hiddenOffsetX = 500.f;   // 화면 오른쪽 밖으로 밀어낼 픽셀 양
    QUEST             m_lastQuestState = QUEST_INACTIVE;

    _bool  m_cachedBasePos = false;
    _float m_baseX_backplate = 0.f;
    _float m_baseX_npc_unknown = 0.f;
    _float m_baseX_npc_female = 0.f;
    _float m_baseX_npc_male = 0.f;
    _float m_baseX_npc_unknown_text = 0.f;
    _float m_baseX_npc_female_text = 0.f;
    _float m_baseX_npc_male_text = 0.f;
    _float m_baseX_quest_text = 0.f;
    _float m_baseX_contents_title = 0.f;
    _float m_baseX_contents_text = 0.f;
};

NS_END