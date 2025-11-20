#include "UILabel.h"

#include "EngineUtility.h"
#include "Transform.h"
#include "UIButton.h"

UILabel::UILabel()
    : UI{}
{
}

UILabel::UILabel(const UILabel& Prototype)
    : UI{ Prototype }
{
}

HRESULT UILabel::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype())) 
        return E_FAIL;

    return S_OK;
}

HRESULT UILabel::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void UILabel::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void UILabel::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void UILabel::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);
}

HRESULT UILabel::Render()
{
    if (m_desc.visible == false)
        return S_OK;

    if (FAILED(__super::Render()))
        return E_FAIL;

    if (m_desc.text.empty())
        return S_OK;

    if (m_desc.font.empty())
        m_desc.font = L"Font_Default";

    m_pEngineUtility->DrawFont(m_desc.font, m_desc.text, _float2(m_desc.x, m_desc.y), XMLoadFloat4(&m_desc.fontColor), m_desc.fontSize);

    return S_OK;
}

UILabel* UILabel::Create()
{
    UILabel* pInstance = new UILabel();
    if (FAILED(pInstance->InitializePrototype()))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

Object* UILabel::Clone(void* pArg)
{
    UILabel* pInstance = new UILabel(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void UILabel::Free()
{
    __super::Free();
}

void UILabel::SetText(const wstring& text)
{
    if (text.empty())
    {
        m_desc.text.clear();
        return;
    }
    m_desc.text = text;
}

const wstring& UILabel::GetText() const
{
    return m_desc.text;
}
