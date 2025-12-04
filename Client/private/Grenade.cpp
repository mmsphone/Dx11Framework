#include "Grenade.h"

#include "EngineUtility.h"
#include "Layer.h"
#include "Player.h"
#include "Drone.h"
#include "Worm.h"
#include "Shieldbug.h"
#include "BloodHitEffect.h"
#include "ExplosionEffect.h"

Grenade::Grenade()
    : ObjectTemplate{}
{
}

Grenade::Grenade(const Grenade& Prototype)
    : ObjectTemplate{ Prototype }
{
}

HRESULT Grenade::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    GRENADE_DESC* pDesc = reinterpret_cast<GRENADE_DESC*>(pArg);
    if (pDesc)
    {
        Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));

        pTransform->RotateRadian(_vector{0.f,0.f,1.f,0.f}, XM_PIDIV2);
        // 시작 위치
        pTransform->SetState(MATRIXROW_POSITION, XMLoadFloat3(&pDesc->vStartPos));

        // 던지는 방향 기반 초기 속도
        _vector vDir = XMLoadFloat3(&pDesc->vDir);  // 정규화 전제
        _vector vForward = XMVector3Normalize(vDir);
        _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

        m_vVelocity = vForward * m_speedForward + vUp * m_speedUp;
    }

    return S_OK;
}

void Grenade::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
    if (IsDead())
        return;

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pTransform)
        return;

    // 수명 증가
    m_lifeTime += fTimeDelta;

    // 중력 적용 (y축)
    _float gravityStep = m_gravity * fTimeDelta;
    m_vVelocity = XMVectorSetY(m_vVelocity, XMVectorGetY(m_vVelocity) - gravityStep);

    // ===== 공중에서 수평 속도(전방 포함) 계속 감속 =====
    {
        // Y 성분 제거 → XZ 성분만 가져옴
        _vector velXZ = XMVectorSetY(m_vVelocity, 0.f);

        // 1초에 40% 정도 줄어드는 느낌 (원하는 감각에 맞게 숫자만 조절)
        _float drag = 1.f - m_airDragPerSec * fTimeDelta;
        if (drag < 0.f) drag = 0.f;

        velXZ *= drag;

        // 다시 Y 성분 붙여서 m_vVelocity 재구성
        _float vy = XMVectorGetY(m_vVelocity);
        m_vVelocity = velXZ + XMVectorSet(0.f, vy, 0.f, 0.f);
    }

    // ---- 이동 전/후 위치 계산 ----
    _vector prevPos = pTransform->GetState(MATRIXROW_POSITION);
    _vector newPos = prevPos + m_vVelocity * fTimeDelta;

    // ---- 네비 위에서 지면 튕김 체크 ----
    _int cellIndex = -1;
    if (m_pEngineUtility->IsInCell(newPos, &cellIndex))
    {
        // 현재 XZ 기준 네비 지면 "기본" 높이
        _float navBaseY = m_pEngineUtility->GetHeightPosOnCell(&newPos, cellIndex);

        // ★ 실제로는 지면보다 m_groundOffset 만큼 위를 기준으로 사용
        _float navY = navBaseY + m_groundOffset;

        _float prevY = XMVectorGetY(prevPos);
        _float nextY = XMVectorGetY(newPos);

        // 이전 위치는 기준 높이 위(or 같음)인데 새 위치가 그 아래로 내려간 경우 → 바닥 충돌
        if (prevY >= navY && nextY < navY)
        {
            // 지면 위로 스냅
            _vector fixedPos{};
            m_pEngineUtility->SetHeightOnCell(newPos, &fixedPos);

            // ★ 여기서도 기준 높이 + 오프셋으로 맞춰줌
            fixedPos = XMVectorSetY(
                fixedPos,
                XMVectorGetY(fixedPos) + m_groundOffset
            );
            newPos = fixedPos;

            // Y속도 반전 + 감쇠
            _float vy = XMVectorGetY(m_vVelocity);
            vy = -vy * m_restitutionY;

            // 수평 속도 감쇠
            _vector velXZ = XMVectorSetY(m_vVelocity, 0.f);
            velXZ *= m_restitutionXZ;

            m_vVelocity = velXZ + XMVectorSet(0.f, vy, 0.f, 0.f);

            // 너무 느려지면 바닥에 거의 붙은 상태로 멈추게
            if (fabsf(vy) < 0.5f)
            {
                vy = 0.f;
                m_vVelocity = XMVectorSetY(velXZ, 0.f); // 완전 평면 이동만 남기거나 아예 0으로
            }
        }
    }

    // 최종 위치 반영
    pTransform->SetState(MATRIXROW_POSITION, newPos);

    // 퓨즈 지나면 폭발 처리
    if (!m_exploded && m_lifeTime >= m_fuseTime)
    {
        Explode();
        return;
    }
}

void Grenade::LateUpdate(_float fTimeDelta)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (pTransform && m_pEngineUtility->IsIn_Frustum_WorldSpace(pTransform->GetState(MATRIXROW_POSITION), m_scaleOffset))
    {
        m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_NONBLEND, this);
        m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_SHADOWLIGHT, this);
    }

    __super::LateUpdate(fTimeDelta);
}

HRESULT Grenade::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix",
        m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix",
        m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_CamFarDistance",
        m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float))))
        return E_FAIL;

    _float alphaMul = 1.f;
    pShader->BindRawValue("g_AlphaMul", &alphaMul, sizeof(_float));

    _uint iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(pModel->BindRenderTargetShaderResource(
            i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
            continue;

        pShader->Begin(0);
        pModel->Render(i);
    }

#ifdef _DEBUG
    if (Collision* pCollision = dynamic_cast<Collision*>(FindComponent(TEXT("Collision"))))
        pCollision->Render();
#endif

    return S_OK;
}

Grenade* Grenade::Create()
{
    Grenade* pInstance = new Grenade();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Grenade");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* Grenade::Clone(void* pArg)
{
    Grenade* pInstance = new Grenade(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Grenade");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Grenade::Free()
{
    __super::Free();
}

HRESULT Grenade::ReadyComponents()
{
    // Shader
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    // Model - 실제 수류탄 모델 리소스 이름으로 수정
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Grenade"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

void Grenade::Explode()
{
    if (m_exploded)
        return;
    m_exploded = true;

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pTransform)
    {
        SetDead(true);
        return;
    }

    // 폭발 중심
    _vector vCenter = pTransform->GetState(MATRIXROW_POSITION);
    const _float radius = m_explodeRadius;
    const _float radiusSq = radius * radius;
    const _float damage = m_explodeDamage;

    const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();

    // ★ 공통 처리 람다: 특정 레이어 안의 모든 오브젝트에 대해 거리 체크
    auto ApplyExplosionToLayer = [&](const wchar_t* layerTag)
        {
            Layer* pLayer = m_pEngineUtility->FindLayer(sceneId, layerTag);
            if (!pLayer)
                return;

            list<Object*> objects = pLayer->GetAllObjects();
            for (Object* obj : objects)
            {
                if (!obj || obj->IsDead())
                    continue;

                // Info 없는 애들은 패스
                Info* pInfo = dynamic_cast<Info*>(obj->FindComponent(TEXT("Info")));
                if (!pInfo)
                    continue;

                INFO_DESC desc = pInfo->GetInfo();

                // 위치 가져와서 반경 체크
                Transform* pTargetTf = static_cast<Transform*>(obj->FindComponent(TEXT("Transform")));
                if (!pTargetTf)
                    continue;

                _vector vTargetPos = pTargetTf->GetState(MATRIXROW_POSITION);
                _vector vDiff = vTargetPos - vCenter;
                _float  distSq = XMVectorGetX(XMVector3LengthSq(vDiff));

                if (distSq > radiusSq)
                    continue; // 폭발 반경 밖

                // ----- 거리 기반 데미지 감쇠 -----
                _float dist = sqrtf(distSq);
                _float distNorm = 0.f;
                if (radius > 0.f)
                    distNorm = std::clamp(dist / radius, 0.f, 1.f);

                // 중심: 1.0, 가장자리: 0.25
                const _float minFactor = 0.25f;
                const _float t = 1.f - distNorm;
                const _float damageFactor =
                    minFactor + (1.f - minFactor) * (1 - distNorm * distNorm);
                const _float finalDamage = damage * damageFactor;

                // HP 감소
                if (auto curHpPtr = desc.GetPtr("CurHP"))
                {
                    _float curHp = *std::get_if<_float>(curHpPtr);
                    curHp -= finalDamage;
                    if (curHp < 0.f)
                        curHp = 0.f;
                    desc.SetData("CurHP", curHp);

                    // 맞았다는 플래그가 있으면 켜주기 (있을 때만)
                    if (desc.GetPtr("IsHit"))
                        desc.SetData("IsHit", _bool{ true });

                    pInfo->BindInfoDesc(desc);
                }

                // ====== 넉백 처리 ======
                // 수평 방향 (XZ) 기준으로 밀어냄
                _vector dirXZ = XMVectorSetY(vDiff, 0.f);
                if (!XMVector3Equal(dirXZ, XMVectorZero()))
                {
                    dirXZ = XMVector3Normalize(dirXZ);

                    _float dist = sqrtf(distSq);
                    _float t = 1.f - (dist / radius);          // 중심: 1, 가장자리: 0
                    t = std::clamp(t, 0.f, 1.f);

                    // 가장자리에서도 약간은 밀리게 최소 계수 섞기
                    _float kbFactor = m_kbMinFactor + (1.f - m_kbMinFactor) * t;
                    _float kbPower = m_kbPowerMax * kbFactor;

                    if (auto* pPlayer = dynamic_cast<Player*>(obj))
                    {
                        pPlayer->SetHit(dirXZ, kbPower, m_kbDuration);
                    }
                    else if (auto* pDrone = dynamic_cast<Drone*>(obj))
                    {
                        pDrone->SetHit(dirXZ, kbPower, m_kbDuration);
                    }
                    else if (auto* pWorm = dynamic_cast<Worm*>(obj))
                    {
                        pWorm->SetHit(dirXZ, kbPower, m_kbDuration);
                    }
                    else if (auto* pShield = dynamic_cast<Shieldbug*>(obj))
                    {
                        pShield->SetHit(dirXZ, kbPower, m_kbDuration);
                    }
                }
                {
                    BloodHitEffect::BLOODHITEFFECT_DESC desc{};
                    desc.fLifeTime = 0.2f;
                    desc.bLoop = false;
                    desc.bAutoKill = true;
                    desc.fRotationPerSec = 0.f;
                    desc.fSpeedPerSec = 0.f;
                    XMStoreFloat3(&desc.vCenterWS,static_cast<Transform*>(obj->FindComponent(TEXT("Transform")))->GetState(MATRIXROW_POSITION));
                    desc.baseColor = _float4(0.2f, 1.f, 0.2f, 1.f);

                    m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("BloodHitEffect"), SCENE::GAMEPLAY, TEXT("Effect"), &desc);
                }
            }
        };

    // 실제로 어떤 레이어 이름을 쓰는지는 네 프로젝트에 맞춰서 수정
    // 예시: "Monster", "Drone", "Shieldbug" 등
    ApplyExplosionToLayer(TEXT("Drone"));
    ApplyExplosionToLayer(TEXT("Worm"));
    ApplyExplosionToLayer(TEXT("Shieldbug"));
    ApplyExplosionToLayer(TEXT("Player"));

    {
        ExplosionEffect::EXPLOSIONEFFECT_DESC eDesc{};
        XMStoreFloat3(&eDesc.vCenterWS, vCenter);
        eDesc.fRadius = radius;
        eDesc.fLifeTime = 1.f;
        m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("ExplosionEffect"), SCENE::GAMEPLAY, TEXT("Effect"), &eDesc);
    }
    {
        Engine::CAMERA_SHAKE_DESC shake{};
        XMStoreFloat3(&shake.vCenterWS, vCenter);
        shake.fDuration = 0.25f;
        shake.fInnerRadius = 0.f;
        shake.fOuterRadius = 25.f;

        // 진폭은 나중에 폭발 세기 따라 튜닝
        shake.vAmpPos = _float3(0.3f, 0.1f, 0.15f);
        shake.vAmpRotDeg = _float3(0.6f, 0.6f, 0.6f);
        shake.fFrequency = 14.f;
        shake.bOverride = true;
        m_pEngineUtility->RequestCameraShake(shake);
    }

    m_pEngineUtility->PlaySound2D("FBX_explosion");

    // 수류탄 제거
    SetDead(true);
}
