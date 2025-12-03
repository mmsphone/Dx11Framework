#pragma once

#include "Client_Defines.h"
#include "UI.h"

NS_BEGIN(Engine)
class Scene;
NS_END

NS_BEGIN(Client)

class endingUI final : public UI
{
public:
    enum ENDTYPE : int { END_WIN, END_LOSE};
private:
    endingUI();
    endingUI(const endingUI& Prototype);
    virtual ~endingUI() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    PriorityUpdate(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    void Show(_bool bShow);
    void SetEndingText(const std::string& text, ENDTYPE type);
    void SetCurScene(Scene* pScene);

    static endingUI* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void    Free() override;

private:
    HRESULT ReadyComponents();
    UI* CreateLetterImage(const _wstring& uiName, const _wstring& texturePath, const UI_DESC& baseDesc, _float x, _float y, _float z, _float w, _float h);
    _float GetCharWidthFactor(char c) const;

private:
    _bool m_isVisible = false;

    vector<UI*> m_letterGlowList;
    vector<UI*> m_letterMainList;

    //기본세팅
    _float m_letterHeightRatio = 0.68f;
    _float m_letterWidthRatio = 0.8f;
    _float m_letterSpacingRatio = -0.5f;
    _float m_lineSpacingRatio = 0.57f;

    //일부 문자용
    _float m_thinWidthFactor = 1.4f;
    _float m_wideWidthFactor = 0.7f;
    _float m_spaceWidthFactor = 0.5f;

    //애니메이션
    vector<_float> m_letterBaseW;
    vector<_float> m_letterBaseH;
    _float m_animTime = 0.f;   // Show(true) 이후 누적 시간
    _float m_plateFadeDuration = 2.f;   // 이미지/플레이트 페이드 시간
    _float m_letterStartScale = 8.f;  // 글자 시작 스케일
    _float m_letterAppearInterval = 0.1f;  // 글자 등장 간격
    _float m_letterPopDuration = 0.25f;  // 각 글자의 팝 인 애니메이션 길이

    _float m_plateTargetAlpha = 0.3f;     // textplate 최종 알파
    _float m_imageTargetAlpha = 1.0f;     // ending_image 최종 알파

    _bool  m_canExit = false;
    Scene* m_curScene = { nullptr };
    _bool m_isWin = false;
};

NS_END
