#include "endingUI.h"

#include "EngineUtility.h"
#include "UI.h"
#include "Layer.h"
#include "Player.h"
#include "UIImage.h"
#include "GameScene.h"
#include "Console.h"
#include "minimapUI.h"

endingUI::endingUI()
    : UI{}
{
}

endingUI::endingUI(const endingUI& Prototype)
    : UI{ Prototype }
{
}

HRESULT endingUI::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT endingUI::Initialize(void* pArg)
{
    UI_DESC desc{};
    desc.fRotationPerSec = 0.f;
    desc.fSpeedPerSec = 0.f;
    desc.name = "endingUI";
    desc.visible = false;
    desc.enable = false;
    if (FAILED(__super::Initialize(&desc)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    if (auto* pPlate = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"ending_textplate")))
    {
        m_plateTargetAlpha = 0.3f;
        pPlate->SetAlpha(m_plateTargetAlpha);
    }
    if (auto* pImg = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"ending_image")))
    {
        m_imageTargetAlpha = 1.0f;
        pImg->SetAlpha(m_imageTargetAlpha);
    }

    return S_OK;
}

void endingUI::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void endingUI::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (!m_isVisible)
        return;

    auto clamp01 = [](_float v)
        {
            if (v < 0.f) return 0.f;
            if (v > 1.f) return 1.f;
            return v;
        };

    m_animTime += fTimeDelta;

    // 1) 백그라운드 이미지 / 플레이트 페이드 인 (0 -> target alpha)
    _float tPlate = clamp01(m_animTime / m_plateFadeDuration);

    if (auto* pImg = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"ending_image")))
        pImg->SetAlpha(m_imageTargetAlpha * tPlate);

    if (auto* pPlate = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"ending_textplate")))
        pPlate->SetAlpha(m_plateTargetAlpha * tPlate);

    // 2) 글자 팝인 연출 (플레이트 페이드가 끝난 뒤부터)
    if (m_animTime <= m_plateFadeDuration)
        return;

    _float letterTime = m_animTime - m_plateFadeDuration;

    const size_t letterCount = m_letterMainList.size();
    if (letterCount == 0 || m_letterBaseW.size() != letterCount)
        return;

    for (size_t i = 0; i < letterCount; ++i)
    {
        UI* pGlowUI = m_letterGlowList[i];
        UI* pMainUI = m_letterMainList[i];
        if (!pGlowUI || !pMainUI)
            continue;

        auto* glowImg = dynamic_cast<UIImage*>(pGlowUI);
        auto* mainImg = dynamic_cast<UIImage*>(pMainUI);
        if (!glowImg || !mainImg)
            continue;

        // 이 글자의 시작 시간과 로컬 시간
        _float startTime = (_float)i * m_letterAppearInterval;
        _float local = letterTime - startTime;

        _float scale = 1.f;
        _float alpha = 0.f;

        if (local <= 0.f)
        {
            // 아직 시작 안함
            scale = 1.f;
            alpha = 0.f;
        }
        else if (local >= m_letterPopDuration)
        {
            // 애니메이션 완료
            scale = 1.f;
            alpha = 1.f;
        }
        else
        {
            _float u = clamp01(local / m_letterPopDuration);
            scale = m_letterStartScale + (1.f - m_letterStartScale) * u; // 10 -> 1
            alpha = u;                                                   // 0 -> 1
        }

        // w/h만 스케일, 중심(x,y)는 그대로
        UI_DESC glowDesc = glowImg->GetUIDesc();
        glowDesc.w = m_letterBaseW[i] * scale;
        glowDesc.h = m_letterBaseH[i] * scale;
        glowImg->ApplyUIDesc(glowDesc);
        glowImg->SetAlpha(alpha); // GetAlpha()가 없다면 그냥 alpha

        UI_DESC mainDesc = mainImg->GetUIDesc();
        mainDesc.w = m_letterBaseW[i] * scale;
        mainDesc.h = m_letterBaseH[i] * scale;
        mainImg->ApplyUIDesc(mainDesc);
        mainImg->SetAlpha(alpha);
    }

    if (!m_canExit && letterCount > 0)
    {
        _float lastStart = (letterCount - 1) * m_letterAppearInterval;
        _float totalLetterAnim = lastStart + m_letterPopDuration;

        if (letterTime >= totalLetterAnim)
            m_canExit = true;
    }

    if (m_canExit && m_curScene && m_pEngineUtility->IsKeyPressed(DIK_RETURN))
    {
        static_cast<GameScene*>(m_curScene)->EndScene(true);
    }
}

void endingUI::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);
}

HRESULT endingUI::Render()
{
    return __super::Render();
}

void endingUI::Show(_bool bShow)
{
    m_isVisible = bShow;
    m_canExit = false;

    Object* pPlayer = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), L"Player", 0);
    static_cast<Player*>(pPlayer)->SetPlayingMinigame(bShow);

    if (!bShow)
    {
        // 끌 때는 그냥 다 숨기기만
        if (UI* p = m_pEngineUtility->FindUI(L"ending_image"))
            p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"ending_textplate"))
            p->SetVisible(false);

        for (UI* p : m_letterGlowList) if (p) p->SetVisible(false);
        for (UI* p : m_letterMainList) if (p) p->SetVisible(false);
        return;
    }

    Info* pInfo = static_cast<Info*>(pPlayer->FindComponent(L"Info"));
    pInfo->GetInfo().SetData("InvincibleLeft", _float{ 999.f });
    static_cast<Console*>(m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Console", 0))->StopWave();
    static_cast<minimapUI*>(m_pEngineUtility->FindUI(L"minimapUI"))->StopMinimap();

    m_animTime = 0.f;

    // 이미지/플레이트는 알파 0부터 시작
    if (auto* pImg = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"ending_image")))
    {
        pImg->SetVisible(true);
        pImg->SetAlpha(0.f);
    }
    if (auto* pPlate = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"ending_textplate")))
    {
        pPlate->SetVisible(true);
        pPlate->SetAlpha(0.f);
    }

    // 글자들 기본 상태 : 보이긴 하되 알파 0
    for (UI* p : m_letterGlowList)
    {
        if (auto* img = dynamic_cast<UIImage*>(p))
        {
            p->SetVisible(true);
            img->SetAlpha(0.f);
        }
    }
    for (UI* p : m_letterMainList)
    {
        if (auto* img = dynamic_cast<UIImage*>(p))
        {
            p->SetVisible(true);
            img->SetAlpha(0.f);
        }
    }

    m_pEngineUtility->FindUI(L"Mouse")->SetVisible(false);
    m_pEngineUtility->FindUI(L"MouseBulletCount")->SetVisible(false);

    if (m_isWin == true)
        m_pEngineUtility->PlaySound2D("FBX_gameclear");
    else
    {
        m_pEngineUtility->PlaySound2D("FBX_gameover");
        _float r = m_pEngineUtility->Random(0, 2);
        if( r >= 1)
            m_pEngineUtility->PlaySound2D("FBX_playerDie1");
        else
            m_pEngineUtility->PlaySound2D("FBX_playerDie2");
    }

    m_pEngineUtility->StopSound("BGM_wave");
    m_pEngineUtility->StopSound("BGM_waveBeep");
    m_pEngineUtility->StopSound("BGM_game");
}

void endingUI::SetEndingText(const std::string& text, ENDTYPE type)
{
    UI* pPlate = m_pEngineUtility->FindUI(L"ending_textplate");
    if (!pPlate)
        return;

    if (type == END_WIN)
        m_isWin = true;
    else
        m_isWin = false;

    UI_DESC plateDesc = pPlate->GetUIDesc();

    // 기존 글자 숨기고 리스트 정리
    for (UI* p : m_letterGlowList)
        if (p) p->SetVisible(false);
    for (UI* p : m_letterMainList)
        if (p) p->SetVisible(false);

    m_letterGlowList.clear();
    m_letterMainList.clear();
    m_letterBaseW.clear();
    m_letterBaseH.clear();

    if (text.empty())
        return;

    const _float charHeight = plateDesc.h * m_letterHeightRatio;
    const _float glyphWidth = charHeight * m_letterWidthRatio;   // 모든 글자 공통 폭
    const _float baseSpacing = glyphWidth * m_letterSpacingRatio; // 기본 간격(음수 가능)
    const _float lineStep = charHeight * m_lineSpacingRatio;

    // ---------------------------
    // 1) 라인별 전체 폭 계산
    //    (두 글자 팩터 평균을 이용해 spacing 계산)
    // ---------------------------
    std::vector<_float> lineWidths;
    _float currentWidth = 0.f;

    auto FlushLine = [&]()
        {
            if (currentWidth <= 0.f)
                return;

            lineWidths.push_back(currentWidth);
            currentWidth = 0.f;
        };

    bool  hasPrev = false;
    float prevFactor = 1.f;
    float prevGlyphWidth = glyphWidth; // 현재는 고정이지만 확장 고려해서 둠

    for (size_t i = 0; i < text.size(); ++i)
    {
        char c = text[i];

        if (c == '\n')
        {
            FlushLine();
            hasPrev = false;
            prevFactor = 1.f;
            prevGlyphWidth = glyphWidth;
            continue;
        }

        float curGlyphWidth = glyphWidth;
        float curFactor = 1.f;

        if (c == ' ')
        {
            // 공백 폭
            curGlyphWidth = glyphWidth * m_spaceWidthFactor;
            curFactor = 1.f; // 공백은 커닝 가중치는 1로 둠 (원하면 별도 팩터 써도 됨)
        }
        else
        {
            // 렌더 가능한 글자만 폭 계산
            wchar_t dummy[2]{};
            if (!((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9')))
            {
                // 지원 안 하는 문자는 폭만 소비하지 않고 그냥 스킵
                continue;
            }

            curFactor = GetCharWidthFactor(c);
        }

        if (!hasPrev)
        {
            // 첫 글자(또는 첫 공백)는 spacing 없이 폭만 더함
            currentWidth += curGlyphWidth;
            hasPrev = true;
        }
        else
        {
            // prev / current 팩터 평균으로 spacing 결정
            const float spacing = baseSpacing * 0.5f * (prevFactor + curFactor);
            currentWidth += spacing + curGlyphWidth;
        }

        prevFactor = curFactor;
        prevGlyphWidth = curGlyphWidth;
    }

    FlushLine();

    if (lineWidths.empty())
        return;

    // ---------------------------
    // 2) 라인별 시작 X/Y 계산
    // ---------------------------
    const _uint  numLines = (_uint)lineWidths.size();
    const _float centerX = plateDesc.x;
    const _float baseY = plateDesc.y - lineStep * ((_float)numLines - 1.f) * 0.5f;

    // ---------------------------
    // 3) 실제 글자 배치 (폭 계산과 동일한 로직으로 진행)
    // ---------------------------
    _uint  lineIndex = 0;
    _float curY = baseY;
    _float lineLeft = centerX - lineWidths[0] * 0.5f;

    hasPrev = false;
    prevFactor = 1.f;
    prevGlyphWidth = glyphWidth;

    float widthAccum = 0.f; // 현재 라인에서 쌓인 폭

    for (size_t i = 0; i < text.size(); ++i)
    {
        char c = text[i];

        if (c == '\n')
        {
            // 다음 라인으로
            ++lineIndex;
            if (lineIndex >= numLines)
                break;

            curY = baseY + lineStep * (_float)lineIndex;
            lineLeft = centerX - lineWidths[lineIndex] * 0.5f;
            widthAccum = 0.f;
            hasPrev = false;
            prevFactor = 1.f;
            prevGlyphWidth = glyphWidth;
            continue;
        }

        float curGlyphWidth = glyphWidth;
        float curFactor = 1.f;
        bool  renderGlyph = true;

        if (c == ' ')
        {
            curGlyphWidth = glyphWidth * m_spaceWidthFactor;
            curFactor = 1.f;
            renderGlyph = false; // 공백은 그리지 않음
        }
        else
        {
            if (!((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9')))
            {
                // 지원 안 하는 문자는 그냥 스킵 (폭도 안 먹음)
                continue;
            }

            curFactor = GetCharWidthFactor(c);
        }

        if (!hasPrev)
        {
            widthAccum += curGlyphWidth;
            hasPrev = true;
        }
        else
        {
            const float spacing = baseSpacing * 0.5f * (prevFactor + curFactor);
            widthAccum += spacing + curGlyphWidth;
        }

        const float glyphCenterX = lineLeft + widthAccum - curGlyphWidth * 0.5f;

        if (renderGlyph)
        {
            // 텍스처 이름 구성
            wchar_t letterName[64] = {};
            if (c >= 'A' && c <= 'Z')
                swprintf_s(letterName, L"letter_%c", c);
            else if (c >= 'a' && c <= 'z')
                swprintf_s(letterName, L"letter_%c", (char)toupper(c));
            else // 숫자
                swprintf_s(letterName, L"letter_%c", c);

            std::wstring basePath = L"../bin/Resources/Textures/Ending/";
            std::wstring mainTex = basePath + letterName + L".png";
            std::wstring glowTex = basePath + letterName + L"_glow.png";

            wchar_t uiNameGlow[64];
            wchar_t uiNameMain[64];
            swprintf_s(uiNameGlow, L"ending_letter_glow_%zu", i);
            swprintf_s(uiNameMain, L"ending_letter_main_%zu", i);

            UI* pGlow = CreateLetterImage(
                uiNameGlow,
                glowTex,
                plateDesc,
                glyphCenterX,
                curY,
                0.f,
                curGlyphWidth,
                charHeight
            );
            if (pGlow)
            {
                m_letterGlowList.push_back(pGlow);
                if (auto* img = dynamic_cast<UIImage*>(pGlow))
                {
                    img->SetAlpha(0.f); // 애니메이션 시작 알파
                    _float4 maskColor = (type == END_WIN)
                        ? _float4{ 0.4f, 1.f, 1.f, 1.f }
                    : _float4{ 1.f, 0.4f, 0.4f, 1.f };
                    img->SetMaskingColor(maskColor);
                }
            }

            UI* pMain = CreateLetterImage(
                uiNameMain,
                mainTex,
                plateDesc,
                glyphCenterX,
                curY,
                0.f,
                curGlyphWidth,
                charHeight
            );
            if (pMain)
            {
                m_letterMainList.push_back(pMain);
                if (auto* img = dynamic_cast<UIImage*>(pMain))
                    img->SetAlpha(0.f); // 애니메이션 시작 알파
            }

            if (pGlow && pMain)
            {
                m_letterBaseW.push_back(curGlyphWidth);
                m_letterBaseH.push_back(charHeight);
            }
        }

        prevFactor = curFactor;
        prevGlyphWidth = curGlyphWidth;
    }
}

void endingUI::SetCurScene(Scene* pScene)
{
    m_curScene = pScene;
}

endingUI* endingUI::Create()
{
    endingUI* pInstance = new endingUI;

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : endingUI");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* endingUI::Clone(void* pArg)
{
    endingUI* pInstance = new endingUI(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : endingUI");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void endingUI::Free()
{
    __super::Free();
}

HRESULT endingUI::ReadyComponents()
{
    return S_OK;
}

UI* endingUI::CreateLetterImage(const _wstring& uiName, const _wstring& texturePath, const UI_DESC& baseDesc, _float x, _float y, _float z, _float w, _float h)
{
    UI_DESC desc = baseDesc;
    desc.x = x;
    desc.y = y;
    desc.w = w;
    desc.h = h;
    desc.z = baseDesc.z + z;
    desc.visible = m_isVisible;
    desc.enable = false;
    desc.imagePath = texturePath;
    m_pEngineUtility->AddObject( SCENE::STATIC, TEXT("UIImage"), SCENE::GAMEPLAY, L"UI", &desc);

    UI* pUI = static_cast<UI*>(m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects().back());
    if (!pUI)
        return nullptr;
    m_pEngineUtility->AddUI(uiName, pUI);

    return pUI;
}

_float endingUI::GetCharWidthFactor(char c) const
{
    if (c >= 'a' && c <= 'z')
        c = (char)toupper(c);

    // 얇은 글자들
    if (c == 'I' || c == '1')
        return m_thinWidthFactor;

    // 넓은 글자들
    if (c == 'M' || c == 'W')
        return m_wideWidthFactor;

    // 공백
    if (c == ' ')
        return m_spaceWidthFactor;

    return 1.f;
}
