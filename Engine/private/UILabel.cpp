#include "UILabel.h"

#include "EngineUtility.h"
#include "Transform.h"

UILabel::UILabel()
    : UI{}
{
}

UILabel::UILabel(const UILabel& Prototype)
    : UI{ Prototype }
    , m_text{ Prototype.m_text }
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

    if (FAILED(ReadyComponents()))
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
    if (FAILED(__super::Render()))
        return E_FAIL;

    m_pEngineUtility->DrawFont(L"Font_Default", m_text, _float2(m_fX, m_fY));

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

void UILabel::SetText(const std::string& text)
{
    if (text.empty())
    {
        m_text.clear();
        return;
    }
    int len = MultiByteToWideChar(CP_UTF8,0,text.c_str(),-1,nullptr,0);
    if (len <= 0)
    {
        m_text.assign(text.begin(), text.end());
        return;
    }

    wstring wide;
    wide.resize(len - 1);
    MultiByteToWideChar(CP_UTF8,0,text.c_str(),-1,&wide[0],len);

    m_text = std::move(wide);
}

const wstring& UILabel::GetText() const
{
    return m_text;
}

HRESULT UILabel::ReadyComponents()
{
    return S_OK;
}