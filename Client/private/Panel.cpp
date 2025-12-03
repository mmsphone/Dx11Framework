#include "Panel.h"

#include "EngineUtility.h"
#include "Door.h"
#include "UI.h"
#include "Layer.h"
#include "hackingGameUI.h"

Panel::Panel()
    : ObjectTemplate{ }
{
}

Panel::Panel(const Panel& Prototype)
    : ObjectTemplate{ Prototype }
{
}

void Panel::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));

    if (m_worked == false || openedHackingUI == true)
    {
        m_playerInRange = false;
        const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
        Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0);
        if (pPlayer)
        {
            Collision* pPlayerCol = static_cast<Collision*>(pPlayer->FindComponent(TEXT("Collision")));
            if (pPlayerCol && pCollision->Intersect(pPlayerCol))
            {
                if (m_pTargetDoor && !m_pEngineUtility->FindUI(L"hacking_text_plate")->IsVisible())
                {
                    openedHackingUI = true;
                    SetVisiblePanelUI(true);
                }
                m_playerInRange = true;
                if (m_pEngineUtility->IsKeyPressed(DIK_E))
                {
                    if (m_pTargetDoor->IsLocked())
                    {
                        if (m_pEngineUtility->FindUI(L"hackingGameUI") == nullptr)
                        {
                            m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("hackingGameUI"), SCENE::GAMEPLAY, L"UI");
                            UI* pUI = static_cast<UI*>(m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects().back());
                            m_pEngineUtility->AddUI(L"hackingGameUI", pUI);
                            static_cast<hackingGameUI*>(pUI)->SetPanel(this);
                        }
                        else
                        {
                            static_cast<hackingGameUI*>(m_pEngineUtility->FindUI(L"hackingGameUI"))->ShowGame();
                        }
                    }
                    else
                        OpenDoor();
                }

                //Å° ¹ÝÂ¦ÀÓ
                if (openedHackingUI)
                {
                    m_keyBlinkAcc += fTimeDelta;
                    if (m_keyBlinkAcc >= 1.f)
                    {
                        m_keyBlinkAcc = 0.f;
                        m_keyBlinkOnState = !m_keyBlinkOnState;

                        m_pEngineUtility->FindUI(L"hacking_key_image_default")->SetVisible(!m_keyBlinkOnState);
                        m_pEngineUtility->FindUI(L"hacking_key_image_on")->SetVisible(m_keyBlinkOnState);
                    }
                }
            }
            else if (openedHackingUI)
            {
                openedHackingUI = false;
                SetVisiblePanelUI(false);
            }
        }
    }
}

void Panel::LateUpdate(_float fTimeDelta)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (m_pEngineUtility->IsIn_Frustum_WorldSpace(pTransform->GetState(MATRIXROW_POSITION), scaleOffset))
    {
        m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_NONBLEND, this);
    }

    __super::LateUpdate(fTimeDelta);
}

HRESULT Panel::Render()
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

void Panel::SetDoor(Door* pDoor)
{
    m_pTargetDoor = pDoor;
}

void Panel::UnlockDoor()
{
    m_pTargetDoor->SetLock(false);
}

void Panel::OpenDoor()
{
    if (m_worked)
        return;

    if (m_pTargetDoor)
    {
        m_pTargetDoor->Open();
        m_worked = true;

        {
            Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0);
            QUEST_EVENT ev{};
            ev.type = EVENTTYPE_INTERACT;
            ev.pInstigator = pPlayer;
            ev.pTarget = this;
            ev.tag = m_panelTag;
            m_pEngineUtility->PushEvent(ev);
        }
    }
}

void Panel::SetPanelTag(const _wstring& panelTag)
{
    m_panelTag = panelTag;
}

Panel* Panel::Create()
{
    Panel* pInstance = new Panel();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Panel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* Panel::Clone(void* pArg)
{
    Panel* pInstance = new Panel(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Panel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Panel::Free()
{
    __super::Free();
}

void Panel::SetVisiblePanelUI(_bool bVisible)
{
    if (bVisible == false)
    {
        m_pEngineUtility->FindUI(L"hacking_text_plate")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_image_plate")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_key_plate")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_image_lock")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_image_unlock")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_key_image_default")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_key_image_on")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_text_hacking")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_text_open")->SetVisible(false);
        return;
    }

    m_keyBlinkAcc = 0.f;
    m_keyBlinkOnState = false;

    m_pEngineUtility->FindUI(L"hacking_text_plate")->SetVisible(true);
    m_pEngineUtility->FindUI(L"hacking_image_plate")->SetVisible(true);
    m_pEngineUtility->FindUI(L"hacking_key_plate")->SetVisible(true);
    if (m_pTargetDoor->IsLocked())
    {
        m_pEngineUtility->FindUI(L"hacking_image_lock")->SetVisible(true);
        m_pEngineUtility->FindUI(L"hacking_text_hacking")->SetVisible(true);
        m_pEngineUtility->FindUI(L"hacking_image_unlock")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_text_open")->SetVisible(false);
    }
    else
    {
        m_pEngineUtility->FindUI(L"hacking_image_lock")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_text_hacking")->SetVisible(false);
        m_pEngineUtility->FindUI(L"hacking_image_unlock")->SetVisible(true);
        m_pEngineUtility->FindUI(L"hacking_text_open")->SetVisible(true);
    }
    m_pEngineUtility->FindUI(L"hacking_key_image_default")->SetVisible(true);
    m_pEngineUtility->FindUI(L"hacking_key_image_on")->SetVisible(false);
}

HRESULT Panel::ReadyComponents()
{
    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Panel"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    CollisionBoxOBB::COLLISIONOBB_DESC     OBBDesc{};
    OBBDesc.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
    XMStoreFloat3(&OBBDesc.vExtents, _vector{ 1.f, 1.f, 0.3f } / scaleOffset);
    OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y * 0.7f, 0.f);
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("Collision"), nullptr, &OBBDesc)))
        return E_FAIL;

    return S_OK;
}