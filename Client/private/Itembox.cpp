#include "Itembox.h"

#include "EngineUtility.h"
#include "UI.h"
#include "UiLabel.h"
#include "Player.h"
#include "Info.h"

Itembox::Itembox()
    : ObjectTemplate{}
{
}

Itembox::Itembox(const Itembox& Prototype)
    : ObjectTemplate{ Prototype }
{
}

void Itembox::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    if (!pTransform || !pCollision)
        return;

    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));

    // ─────────────────────────────
    // 이미 사용 후 → 1초 동안 페이드 아웃
    // ─────────────────────────────
    if (m_isDying)
    { 
        m_deathTimer += fTimeDelta;
        float t = m_deathTimer / 1.f; // 1초 기준
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;

        m_deathFade = 1.f - t;        // 1 → 0 감소

        if (m_deathTimer >= 1.f)
        {
            SetDead(true);
        }

        // 죽는 중에는 더 이상 UI, 충돌 처리 안 함
        return;
    }

    m_playerInRange = false;

    const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
    Object* pPlayerObj = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0);
    if (!pPlayerObj)
        return;

    Collision* pPlayerCol = static_cast<Collision*>(pPlayerObj->FindComponent(TEXT("Collision")));
    if (!pPlayerCol)
        return;

    if (pCollision->Intersect(pPlayerCol))
    {
        m_playerInRange = true;

        // 범위 안에 처음 들어왔을 때 UI 켬
        if (!m_openedUI)
        {
            m_openedUI = true;
            SetVisibleItemboxUI(true);
        }

        // E 키 입력 시 아이템 지급, 사용 종료 + 페이드 시작
        if (!m_worked && m_pEngineUtility->IsKeyPressed(DIK_E))
        {
            if (GiveGrenadeToPlayer())
            {
                m_worked = true;

                // UI 끄고
                m_openedUI = false;
                SetVisibleItemboxUI(false);

                // 페이드 시작
                m_isDying = true;
                m_deathFade = 1.f;
                m_deathTimer = 0.f;

                return;
            }
        }

        // 패널/아모백과 동일하게 키 반짝임 처리 (UI 켜져 있을 때만)
        if (m_openedUI)
        {
            m_keyBlinkAcc += fTimeDelta;
            if (m_keyBlinkAcc >= 1.f)
            {
                m_keyBlinkAcc = 0.f;
                m_keyBlinkOnState = !m_keyBlinkOnState;

                UI* pKeyDefault = m_pEngineUtility->FindUI(L"itembox_key_image_default");
                UI* pKeyOn = m_pEngineUtility->FindUI(L"itembox_key_image_on");

                if (pKeyDefault)
                    pKeyDefault->SetVisible(!m_keyBlinkOnState);
                if (pKeyOn)
                    pKeyOn->SetVisible(m_keyBlinkOnState);
            }
        }
    }
    else
    {
        // 범위에서 벗어나면 UI 끔
        if (m_openedUI)
        {
            m_openedUI = false;
            SetVisibleItemboxUI(false);
        }
    }
}

void Itembox::LateUpdate(_float fTimeDelta)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (pTransform && m_pEngineUtility->IsIn_Frustum_WorldSpace(pTransform->GetState(MATRIXROW_POSITION), scaleOffset))
    {
        // 죽는 중이면 BLEND, 아니면 NONBLEND
        if (m_isDying)
            m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_BLEND, this);
        else
            m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_NONBLEND, this);
    }

    __super::LateUpdate(fTimeDelta);
}

HRESULT Itembox::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix",
        m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix",
        m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_CamFarDistance",
        m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float))))
        return E_FAIL;

    // 알파 값
    _float alphaMul = 1.f;
    if (m_isDying)
        alphaMul = m_deathFade;
    if (FAILED(pShader->BindRawValue("g_AlphaMul", &alphaMul, sizeof(_float))))
        return E_FAIL;

    if (m_isDying)
    {
        m_pEngineUtility->BindRenderTargetShaderResource(L"RenderTarget_Shade", pShader, "g_ShadeTexture");
        m_pEngineUtility->BindRenderTargetShaderResource(L"RenderTarget_Specular", pShader, "g_SpecularTexture");
    }

    _uint iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(pModel->BindRenderTargetShaderResource(i, pShader,
            "g_DiffuseTexture", TextureType::Diffuse, 0)))
            continue;

        // 죽는 중이면 ModelAlpha 패스(= index 6), 아니면 기본 GBuffer 패스(= 0)
        if (!m_isDying)
            pShader->Begin(0);
        else
            pShader->Begin(6);

        pModel->Render(i);
    }

#ifdef _DEBUG
    if (Collision* pCollision = dynamic_cast<Collision*>(FindComponent(TEXT("Collision"))))
        pCollision->Render();
#endif

    _float one = 1.f;
    pShader->BindRawValue("g_AlphaMul", &one, sizeof(_float));

    return S_OK;
}

Itembox* Itembox::Create()
{
    Itembox* pInstance = new Itembox();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Itembox");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* Itembox::Clone(void* pArg)
{
    Itembox* pInstance = new Itembox(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Itembox");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Itembox::Free()
{
    __super::Free();
}

HRESULT Itembox::ReadyComponents()
{
    // Shader
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    // Model - 실제 리소스 이름에 맞게 수정
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Itembox"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    // Collision OBB
    CollisionBoxOBB::COLLISIONOBB_DESC OBBDesc{};
    OBBDesc.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
    XMStoreFloat3(&OBBDesc.vExtents, _vector{ 0.6f, 0.6f, 0.6f } / scaleOffset);
    OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y * 0.7f, 0.f);

    if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("Collision"), nullptr, &OBBDesc)))
        return E_FAIL;

    return S_OK;
}

void Itembox::SetVisibleItemboxUI(_bool bVisible)
{
    if (!bVisible)
    {
        m_keyBlinkAcc = 0.f;
        m_keyBlinkOnState = false;

        if (UI* p = m_pEngineUtility->FindUI(L"itembox_text_plate"))        p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"itembox_image_plate"))       p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"itembox_key_plate"))         p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"itembox_image"))             p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"itembox_key_image_default")) p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"itembox_key_image_on"))      p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"itembox_text"))              p->SetVisible(false);
        return;
    }

    m_keyBlinkAcc = 0.f;
    m_keyBlinkOnState = false;

    if (UI* p = m_pEngineUtility->FindUI(L"itembox_text_plate"))       p->SetVisible(true);
    if (UI* p = m_pEngineUtility->FindUI(L"itembox_image_plate"))      p->SetVisible(true);
    if (UI* p = m_pEngineUtility->FindUI(L"itembox_key_plate"))        p->SetVisible(true);
    if (UI* p = m_pEngineUtility->FindUI(L"itembox_image"))            p->SetVisible(true);

    if (UI* p = m_pEngineUtility->FindUI(L"itembox_key_image_default")) p->SetVisible(true);
    if (UI* p = m_pEngineUtility->FindUI(L"itembox_key_image_on"))      p->SetVisible(false);
    if (UI* p = m_pEngineUtility->FindUI(L"itembox_text"))              p->SetVisible(true);
}

_bool Itembox::GiveGrenadeToPlayer()
{
    Object* pPlayerObj = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
    Info* pInfo = static_cast<Info*>(pPlayerObj->FindComponent(L"Info"));

    INFO_DESC desc = pInfo->GetInfo();
    _int grenadeCount = *std::get_if<_int>(desc.GetPtr("GrenadeCount"));
    if (grenadeCount >= 5)
        return false;

    _int newGrenadeCount = grenadeCount + 5;
    newGrenadeCount = clamp(newGrenadeCount, 0, 5);
    desc.SetData("GrenadeCount", _int{ newGrenadeCount });
    pInfo->BindInfoDesc(desc);

    static_cast<UILabel*>(m_pEngineUtility->FindUI(L"grenadeCount"))->SetText(to_wstring(newGrenadeCount));

    {
        QUEST_EVENT ev{};
        ev.type = EVENTTYPE_INTERACT;
        ev.pInstigator = pPlayerObj;
        ev.pTarget = this;
        ev.tag = L"itembox";
        m_pEngineUtility->PushEvent(ev);
    }

    m_pEngineUtility->PlaySound2D("FBX_pickupItembox", 0.7f);
    static_cast<Player*>(pPlayerObj)->SetPickupState();

    return true;
}
