#include "UIButton.h"

#include "EngineUtility.h"
#include "Transform.h"
#include "UILabel.h"

UIButton::UIButton()
    : UI{}
{
}

UIButton::UIButton(const UIButton& Prototype)
    : UI{ Prototype }
    , leftButtonFunctions{}
{
}

HRESULT UIButton::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT UIButton::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void UIButton::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void UIButton::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (!m_desc.enable)
        return;

    if (IsMouseOver())
    {
        if (m_defaultImage && m_onImage)
        {
            m_defaultImage->SetVisible(false);
            m_onImage->SetVisible(true);
        }
        if (m_text)
            static_cast<UILabel*>(m_text)->SetColor(_vector{ 1.f, 1.f, 1.f, 1.f });

        if (m_pEngineUtility->IsMousePressed(MOUSEKEY_LEFTBUTTON))
        {
            DoLeftButtonFunctions();
        }
        if (m_pEngineUtility->IsMousePressed(MOUSEKEY_RIGHTBUTTON))
        {
            DoRightButtonFunctions();
        }
    }
    else
    {
        if (m_defaultImage && m_onImage) {
            m_defaultImage->SetVisible(true);
            m_onImage->SetVisible(false);
        }
        if (m_text)
            static_cast<UILabel*>(m_text)->SetColor(_vector{ 0.6f, 0.6f, 0.6f, 1.f });
    }
}

void UIButton::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);
}

HRESULT UIButton::Render()
{
    if (m_desc.visible == false)
        return S_OK;

    if (FAILED(__super::Render()))
        return E_FAIL;

    return S_OK;
}

void UIButton::SetEnable(_bool bEnable)
{
    m_desc.enable = bEnable;
}

_bool UIButton::IsEnable()
{
    return m_desc.enable;
}

void UIButton::AddLeftButtonFunction(function<void()> func)
{
    if (func)
        leftButtonFunctions.push_back(std::move(func));
}

void UIButton::DoLeftButtonFunctions()
{
    if (m_desc.enable == false)
        return;

    for (auto& func : leftButtonFunctions)
    {
        if (func)
            func();
    }
}

void UIButton::AddRightButtonFunction(function<void()> func)
{
    if (func)
        rightButtonFunctions.push_back(std::move(func));
}

void UIButton::DoRightButtonFunctions()
{
    if (m_desc.enable == false)
        return;

    for (auto& func : rightButtonFunctions)
    {
        if (func)
            func();
    }
}

void UIButton::ClearButtonFunctions()
{
    leftButtonFunctions.clear();
    rightButtonFunctions.clear();
}

void UIButton::SetDefaultImage(const wstring& defaultKey)
{
    m_defaultImage = m_pEngineUtility->FindUI(defaultKey);
    if (m_defaultImage)
        m_defaultImage->SetVisible(true);
}

void UIButton::SetOnImage(const wstring& onKey)
{
    m_onImage = m_pEngineUtility->FindUI(onKey);
    if (m_onImage)
        m_onImage->SetVisible(false);
}

void UIButton::SetText(const wstring& textKey)
{
    m_text = m_pEngineUtility->FindUI(textKey);
    if (m_text)
        m_text->SetVisible(true);
}

_bool UIButton::IsMouseOver() const
{
    if (m_desc.enable == false)
        return false;

    _float2 mousePos = m_pEngineUtility->GetMousePos();

    const _float left = m_desc.x - m_desc.w * 0.5f;
    const _float top = m_desc.y - m_desc.h * 0.5f;
    const _float right = m_desc.x + m_desc.w * 0.5f;
    const _float bottom = m_desc.y + m_desc.h * 0.5f;

    return (mousePos.x >= left && mousePos.x <= right &&
        mousePos.y >= top && mousePos.y <= bottom);
}

UIButton* UIButton::Create()
{
    UIButton* pInstance = new UIButton();
    if (FAILED(pInstance->InitializePrototype()))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

Object* UIButton::Clone(void* pArg)
{
    UIButton* pInstance = new UIButton(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void UIButton::Free()
{
    __super::Free();
}


HRESULT UIButton::ReadyComponents()
{
    return S_OK;
}