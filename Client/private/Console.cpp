#include "Console.h"

#include "EngineUtility.h"

Console::Console()
    : ObjectTemplate{ }
{
}

Console::Console(const Console& Prototype)
    : ObjectTemplate{ Prototype }
{
}

HRESULT Console::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_waveDurations.push_back(0.f);
    m_waveDurations.push_back(40.f);
    m_waveDurations.push_back(30.f);
    m_waveDurations.push_back(20.f);
    m_waveDurations.push_back(10.f);

    return S_OK;
}

void Console::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));

    const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
    Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0);
    if (!m_isActive)
    {
        m_playerInRange = false;
        if (pPlayer)
        {
            Collision* pPlayerCol = static_cast<Collision*>(pPlayer->FindComponent(TEXT("Collision")));
            if (pPlayerCol && pCollision->Intersect(pPlayerCol))
            {
                m_playerInRange = true;

                // F 키 눌렀을 때 웨이브 스폰
                if (m_pEngineUtility->IsKeyPressed(DIK_E))
                {
                    m_isActive = true;
                    m_waveIndex = 0;

                    // 첫 웨이브까지 남은 시간 세팅
                    if (!m_waveDurations.empty())
                        m_waveTimer = m_waveDurations[0];
                    else
                        m_waveTimer = 0.f;

                    // 첫 간격이 0이면 바로 첫 웨이브 스폰
                    if (m_waveTimer <= 0.f)
                    {
                        m_pEngineUtility->Spawn(7);
                        ++m_waveIndex;

                        if (m_waveIndex < m_waveDurations.size())
                            m_waveTimer = m_waveDurations[m_waveIndex];
                    }
                }
            }
        }
    }
    else
    {
        if (m_waveIndex < m_waveDurations.size())
        {
            m_waveTimer -= fTimeDelta;

            if (m_waveTimer <= 0.f)
            {
                // 웨이브 스폰
                m_pEngineUtility->Spawn(7);

                // 다음 웨이브 준비
                ++m_waveIndex;
                if (m_waveIndex < m_waveDurations.size())
                {
                    m_waveTimer = m_waveDurations[m_waveIndex];
                }
                else
                {
                    //웨이브 끝 처리
                }
            }
        }
    }
}

void Console::LateUpdate(_float fTimeDelta)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (m_pEngineUtility->IsIn_Frustum_WorldSpace(pTransform->GetState(MATRIXROW_POSITION), scaleOffset))
    {
        m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_NONBLEND, this);
    }

    __super::LateUpdate(fTimeDelta);
}

HRESULT Console::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_CamFarDistance", m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float))))
        return E_FAIL;

    _uint       iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(pModel->BindRenderTargetShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
            continue;

        pShader->Begin(0);
        pModel->Render(i);
    }

#ifdef _DEBUG
    Collision* pCollision = dynamic_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Render();
#endif

    return S_OK;
}

Console* Console::Create()
{
    Console* pInstance = new Console();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Console");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* Console::Clone(void* pArg)
{
    Console* pInstance = new Console(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Console");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Console::Free()
{
    __super::Free();
}

HRESULT Console::ReadyComponents()
{
    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Console"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    CollisionBoxOBB::COLLISIONOBB_DESC     OBBDesc{};
    OBBDesc.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
    XMStoreFloat3(&OBBDesc.vExtents, _vector{ 1.f, 1.f, 0.40f } / scaleOffset);
    OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y * 0.7f, 0.f);
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("Collision"), nullptr, &OBBDesc)))
        return E_FAIL;

    return S_OK;
}