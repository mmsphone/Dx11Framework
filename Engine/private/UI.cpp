#include "UI.h"
#include "Transform.h"
#include "EngineUtility.h"

UI::UI()
	: Object {}
{
}

UI::UI(const UI& Prototype)
	:Object{ Prototype }
    , m_fViewportSizeX{ Prototype.m_fViewportSizeX }
    , m_fViewportSizeY{ Prototype.m_fViewportSizeY }
    , m_ViewMatrix{ Prototype.m_ViewMatrix }
    , m_ProjMatrix{ Prototype.m_ProjMatrix }
{
}

HRESULT UI::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    _uint                   iNumViewports = { 1 };
    D3D11_VIEWPORT			ViewPortDesc{};
    m_pEngineUtility->GetContext()->RSGetViewports(&iNumViewports, &ViewPortDesc);
    m_fViewportSizeX = ViewPortDesc.Width;
    m_fViewportSizeY = ViewPortDesc.Height;
    XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_fViewportSizeX, m_fViewportSizeY, 0.f, 1.f));

	return S_OK;
}

HRESULT UI::Initialize(void* pArg)
{
    if (pArg == nullptr)
        return E_FAIL;

    UI_DESC* pDesc = static_cast<UI_DESC*>(pArg);
    m_desc = *pDesc;

    Object::OBJECT_DESC desc{};
    desc.fRotationPerSec = m_desc.fRotationPerSec;
    desc.fSpeedPerSec = m_desc.fSpeedPerSec;
	if (FAILED(__super::Initialize(&desc)))
		return E_FAIL;

    UpdateState();

    return S_OK;
}

void UI::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void UI::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void UI::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);

    if (m_desc.visible == false)
        return;

    UpdateState();
    m_pEngineUtility->JoinRenderGroup(RENDER_UI, this);
}

HRESULT UI::Render()
{
    if (m_desc.visible == false)
        return S_OK;

    if (FAILED(__super::Render())) 
        return E_FAIL;

	return S_OK;
}

void UI::ApplyUIDesc(const UI_DESC& desc)
{
    m_desc = desc;
    UpdateState();
}

const UI_DESC& UI::GetUIDesc() const
{
    return m_desc;
}

void UI::SetVisible(_bool bVisible)
{
    m_desc.visible = bVisible;
}

void UI::Free()
{
    __super::Free();
}

void UI::UpdateState()
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    pTransform->SetScale(m_desc.w, m_desc.h);
    pTransform->SetState(MATRIXROW::MATRIXROW_POSITION, XMVectorSet(m_desc.x - m_fViewportSizeX * 0.5f, -m_desc.y + m_fViewportSizeY * 0.5f, m_desc.z, 1.f ));
}
