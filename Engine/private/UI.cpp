#include "UI.h"
#include "Transform.h"
#include "EngineUtility.h"

UI::UI()
	: Object {}
{
}

UI::UI(const UI& Prototype)
	:Object{ Prototype }
{
}

HRESULT UI::InitializePrototype()
{
	return S_OK;
}

HRESULT UI::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

    if (pArg == nullptr)
        return E_FAIL;

    UI_DESC* pDesc = static_cast<UI_DESC*>(pArg);

    m_fX = pDesc->fX;
    m_fY = pDesc->fY;
    m_fZ = pDesc->fZ;
    m_fSizeX = pDesc->fSizeX;
    m_fSizeY = pDesc->fSizeY;

    _uint                   iNumViewports = { 1 };
    D3D11_VIEWPORT			ViewPortDesc{};

    m_pEngineUtility->GetContext()->RSGetViewports(&iNumViewports, &ViewPortDesc);

    m_fViewportSizeX = ViewPortDesc.Width;
    m_fViewportSizeY = ViewPortDesc.Height;

    UpdateState();

    XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_fViewportSizeX, m_fViewportSizeY, 0.f, 1.f));

    return S_OK;
}

void UI::PriorityUpdate(_float fTimeDelta)
{
}

void UI::Update(_float fTimeDelta)
{
}

void UI::LateUpdate(_float fTimeDelta)
{
    UpdateState();

    m_pEngineUtility->JoinRenderGroup(RENDER_UI, this);
}

HRESULT UI::Render()
{
	return S_OK;
}

void UI::ApplyRectFromControl(const UIControlDesc& desc)
{
    m_fX = static_cast<_float>(desc.x);
    m_fY = static_cast<_float>(desc.y);
    m_fSizeX = static_cast<_float>(desc.w);
    m_fSizeY = static_cast<_float>(desc.h);

    UpdateState();
}

void UI::Free()
{
    __super::Free();
}

void UI::UpdateState()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));

    pTransform->SetScale(m_fSizeX, m_fSizeY);
    pTransform->SetState(MATRIXROW::MATRIXROW_POSITION, XMVectorSet(
        m_fX - m_fViewportSizeX * 0.5f,
        -m_fY + m_fViewportSizeY * 0.5f,
        m_fZ,
        1.f
    ));
}
