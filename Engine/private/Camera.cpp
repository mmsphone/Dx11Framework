#include "Camera.h"

#include "EngineUtility.h"

Camera::Camera()
	:Object{}
{
}

Camera::Camera(const Camera& Prototype)
	:Object{ Prototype }
{
}

HRESULT Camera::InitializePrototype()
{
	return S_OK;
}

HRESULT Camera::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    CAMERA_DESC* pDesc = static_cast<CAMERA_DESC*>(pArg);

    m_fFovy = pDesc->fFovy;
    m_fNear = pDesc->fNear;
    m_fFar = pDesc->fFar;

    _uint       iNumViewports = { 1 };
    D3D11_VIEWPORT      ViewportDesc{};

    m_pEngineUtility->GetContext()->RSGetViewports(&iNumViewports, &ViewportDesc);

    m_fAspect = static_cast<_float>(ViewportDesc.Width) / ViewportDesc.Height;

    /* 사용자가 셋팅하고 싶은 카메라의 초기 상태를 트랜스폼에 동기화했다. */
    /* 뷰행렬을 만들기위한 어느정도의 준비는 됐다. */
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    pTransform->SetState(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vEye), 1.f));
    pTransform->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vAt), 1.f));

    /* 초기데이터 파이프라인에 셋팅. */
    UpdatePipeLine();

    return S_OK;
}

void Camera::PriorityUpdate(_float fTimeDelta)
{
}

void Camera::Update(_float fTimeDelta)
{
}

void Camera::LateUpdate(_float fTimeDelta)
{
}

HRESULT Camera::Render()
{
	return S_OK;
}

void Camera::Free()
{
    __super::Free();
}

void Camera::UpdatePipeLine()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    m_pEngineUtility->SetTransform(D3DTS::D3DTS_VIEW, pTransform->GetWorldMatrixInverse());
    m_pEngineUtility->SetTransform(D3DTS::D3DTS_PROJECTION, XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar));
}