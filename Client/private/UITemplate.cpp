#include "UITemplate.h"

#include "EngineUtility.h"

UITemplate::UITemplate()
    :UI{}
{
}

UITemplate::UITemplate(const UITemplate& Prototype)
    :UI{ Prototype }
{
}

HRESULT UITemplate::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT UITemplate::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());

    _float2 viewport = m_pEngineUtility->GetWindowSize();
    m_fViewportSizeX = viewport.x;
    m_fViewportSizeY = viewport.y;
    XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_fViewportSizeX, m_fViewportSizeY, 0.f, 1.f));

    return S_OK;
}

void UITemplate::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::UI, this);

    __super::LateUpdate(fTimeDelta);
}
