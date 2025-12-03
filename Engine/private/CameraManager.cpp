#include "CameraManager.h"

#include "EngineUtility.h"
#include "Camera.h"

CameraManager::CameraManager()
    : Base{}
    , m_pEngineUtility{ EngineUtility::GetInstance() }
{
    SafeAddRef(m_pEngineUtility);
}

HRESULT CameraManager::Initialize()
{
    m_bShakeActive = false;
    m_fShakeElapsed = 0.f;
    m_pMainCamera = nullptr;
    m_Cameras.clear();
    return S_OK;
}

CameraManager* CameraManager::Create()
{
    CameraManager* pInstance = new CameraManager();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CameraManager");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void CameraManager::Free()
{
    m_Cameras.clear();
    m_pMainCamera = nullptr;
    SafeRelease(m_pEngineUtility);

    __super::Free();
}

void CameraManager::RegisterCamera(Camera* pCamera)
{
    if (!pCamera)
        return;

    auto it = std::find(m_Cameras.begin(), m_Cameras.end(), pCamera);
    if (it == m_Cameras.end())
        m_Cameras.push_back(pCamera);

    // 첫 등록 카메라를 자동으로 메인으로 잡고 싶으면 여기서:
    if (!m_pMainCamera)
        m_pMainCamera = pCamera;
}

void CameraManager::UnregisterCamera(Camera* pCamera)
{
    if (!pCamera)
        return;

    auto it = std::find(m_Cameras.begin(), m_Cameras.end(), pCamera);
    if (it != m_Cameras.end())
        m_Cameras.erase(it);

    if (m_pMainCamera == pCamera)
        m_pMainCamera = m_Cameras.empty() ? nullptr : m_Cameras.front();
}

void CameraManager::SetMainCamera(Camera* pCamera)
{
    m_pMainCamera = pCamera;
}
class Camera* CameraManager::GetMainCamera() const
{
    return m_pMainCamera;
}

void CameraManager::RequestCameraShake(const CAMERA_SHAKE_DESC& desc)
{
    if (m_bShakeActive && !desc.bOverride)
        return;

    m_ShakeDesc = desc;
    m_fShakeElapsed = 0.f;
    m_bShakeActive = true;
}

void CameraManager::Update(_float fTimeDelta)
{
    if (!m_pMainCamera)
        return;

    Transform* pTransform = static_cast<Transform*>(
        m_pMainCamera->FindComponent(TEXT("Transform")));
    if (!pTransform)
        return;

    // 기본 View
    _matrix baseView = pTransform->GetWorldMatrixInverse();

    // 쉐이크 적용
    _matrix view = ApplyShakeToView(baseView, fTimeDelta);

    const _float fFovy = m_pMainCamera->GetFovy();
    const _float fAspect = m_pMainCamera->GetAspect();
    const _float fNear = m_pMainCamera->GetNear();
    const _float fFar = m_pMainCamera->GetFar();
    _matrix proj = XMMatrixPerspectiveFovLH(fFovy, fAspect, fNear, fFar);

    m_pEngineUtility->SetPipelineTransform(D3DTS::D3DTS_VIEW, view);
    m_pEngineUtility->SetPipelineTransform(D3DTS::D3DTS_PROJECTION, proj);
    m_pEngineUtility->SetPipelineFarDistance(fFar);

    //사운드
    _vector posV = pTransform->GetState(MATRIXROW_POSITION);
    _vector lookV = pTransform->GetState(MATRIXROW_LOOK);
    _vector upV = pTransform->GetState(MATRIXROW_UP);

    _float3 pos{}, forward{}, up{};
    XMStoreFloat3(&pos, posV);
    XMStoreFloat3(&forward, XMVector3Normalize(lookV));
    XMStoreFloat3(&up, XMVector3Normalize(upV));

    _float3 vel{ 0.f, 0.f, 0.f }; // 원하면 직접 속도 계산해서 넣기

    m_pEngineUtility->SetSoundListener(pos, forward, up, vel);
}

_matrix CameraManager::ApplyShakeToView(_fmatrix baseView, _float fTimeDelta)
{
    if (!m_bShakeActive)
        return baseView;

    m_fShakeElapsed += fTimeDelta;

    const _float duration = XMMax(m_ShakeDesc.fDuration, 0.0001f);
    _float t = m_fShakeElapsed / duration;
    if (t >= 1.f)
    {
        m_bShakeActive = false;
        return baseView;
    }

    // 시간 감쇠
    _float timeAtten = 1.f - t;

    // baseView -> 카메라 월드 행렬
    XMMATRIX xmBaseView = baseView;
    XMMATRIX xmCamWorld = XMMatrixInverse(nullptr, xmBaseView);

    // 카메라 위치
    _float3 camPos{};
    XMStoreFloat3(&camPos, xmCamWorld.r[3]);

    // 거리 감쇠
    XMVECTOR vCam = XMLoadFloat3(&camPos);
    XMVECTOR vCenter = XMLoadFloat3(&m_ShakeDesc.vCenterWS);
    _float dist = XMVectorGetX(XMVector3Length(vCam - vCenter));

    _float distAtten = 1.f;
    if (m_ShakeDesc.fOuterRadius > 0.f)
    {
        if (dist >= m_ShakeDesc.fOuterRadius)
            return baseView;

        const _float inner = XMMax(0.f, m_ShakeDesc.fInnerRadius);
        const _float outer = m_ShakeDesc.fOuterRadius;

        if (dist > inner)
        {
            const _float len = XMMax(outer - inner, 0.0001f);
            distAtten = 1.f - (dist - inner) / len;
        }
        else
        {
            distAtten = 1.f;
        }
    }

    _float finalAtten = timeAtten * distAtten;
    if (finalAtten <= 0.f)
        return baseView;

    const _float w = m_ShakeDesc.fFrequency * XM_2PI;
    const _float tt = m_fShakeElapsed;

    _float3 offPos
    {
        m_ShakeDesc.vAmpPos.x * finalAtten * sinf(w * tt + 0.0f),
        m_ShakeDesc.vAmpPos.y * finalAtten * sinf(w * tt + 2.0f),
        m_ShakeDesc.vAmpPos.z * finalAtten * sinf(w * tt + 4.0f)
    };

    _float3 rotDeg
    {
        m_ShakeDesc.vAmpRotDeg.x * finalAtten * sinf(w * tt + 1.0f),
        m_ShakeDesc.vAmpRotDeg.y * finalAtten * sinf(w * tt + 3.0f),
        m_ShakeDesc.vAmpRotDeg.z * finalAtten * sinf(w * tt + 5.0f)
    };

    XMMATRIX rotShake =
        XMMatrixRotationX(XMConvertToRadians(rotDeg.x)) *
        XMMatrixRotationY(XMConvertToRadians(rotDeg.y)) *
        XMMatrixRotationZ(XMConvertToRadians(rotDeg.z));

    // 회전 + 위치 오프셋 적용
    XMVECTOR trans = xmCamWorld.r[3];
    trans = XMVectorAdd(trans, XMVectorSet(offPos.x, offPos.y, offPos.z, 0.f));

    xmCamWorld = rotShake * xmCamWorld;
    xmCamWorld.r[3] = trans;

    XMMATRIX xmViewShaken = XMMatrixInverse(nullptr, xmCamWorld);
    return xmViewShaken;
}
