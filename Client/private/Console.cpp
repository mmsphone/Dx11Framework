#include "Console.h"

#include "EngineUtility.h"
#include "UI.h"
#include "UIImage.h"
#include "hacking2GameUI.h"
#include "Layer.h"
#include "Player.h"

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
    m_waveDurations.push_back(14.f);
    m_waveDurations.push_back(12.f);
    m_waveDurations.push_back(10.f);
    m_waveDurations.push_back(8.f);
    m_waveDurations.push_back(6.f);
    m_waveDurations.push_back(4.f);

    m_waveIndex = 0;
    m_waveTimer = 0.f;
    m_currentWaveDuration = 0.f;

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

    m_playerInRange = false;

    if (pPlayer)
    {
        Collision* pPlayerCol = static_cast<Collision*>(pPlayer->FindComponent(TEXT("Collision")));
        if (pPlayerCol && pCollision->Intersect(pPlayerCol))
        {
            m_playerInRange = true;

            // 처음 들어왔을 때 hacking2_ UI 띄우기
            UI* pTextPlate = m_pEngineUtility->FindUI(L"hacking2_text_plate");
            if (pTextPlate && !pTextPlate->IsVisible())
            {
                m_openedHackingUI = true;
                ReadyWaveUI();
                SetVisibleConsoleUI(true);
            }

            // E 키 → 웨이브 시작 + hackingGame2UI 실행
            if (m_pEngineUtility->IsKeyPressed(DIK_E))
            {
                // 1) 웨이브 시퀀스 시작 (이미 진행 중이면 내부에서 무시)
                StartWaveSequence();

                // 2) 미니게임 UI 생성 및 보여주기
                if (m_pEngineUtility->FindUI(L"hacking2GameUI") == nullptr)
                {
                    // Object 생성
                    m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("hacking2GameUI"), SCENE::GAMEPLAY, L"UI");
                
                    // UI 레이어에서 마지막 Object 가져와서 UI로 등록
                    UI* pUI = static_cast<UI*>(m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects().back());
                    m_pEngineUtility->AddUI(L"hacking2GameUI", pUI);
                
                    static_cast<hacking2GameUI*>(pUI)->SetConsole(this);
                }
                else
                {
                    static_cast<hacking2GameUI*>(m_pEngineUtility->FindUI(L"hacking2GameUI"))->ShowGame();
                }
            }

            // 키 깜빡임
            if (m_openedHackingUI)
            {
                m_keyBlinkAcc += fTimeDelta;
                if (m_keyBlinkAcc >= 1.f)
                {
                    m_keyBlinkAcc = 0.f;
                    m_keyBlinkOnState = !m_keyBlinkOnState;

                    if (UI* pKeyDefault = m_pEngineUtility->FindUI(L"hacking2_key_image_default"))
                        pKeyDefault->SetVisible(!m_keyBlinkOnState);
                    if (UI* pKeyOn = m_pEngineUtility->FindUI(L"hacking2_key_image_on"))
                        pKeyOn->SetVisible(m_keyBlinkOnState);
                }
            }
        }
        else if (m_openedHackingUI)
        {
            // 범위 밖으로 나가면 콘솔 UI 닫기
            m_openedHackingUI = false;
            SetVisibleConsoleUI(false);
        }
    }

    // === 웨이브 진행 로직 (기존 것 유지) ===
    if (m_isActive && m_waveIndex < m_waveDurations.size())
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
                m_currentWaveDuration = m_waveDurations[m_waveIndex];
            }
            else
            {
                // 모든 웨이브 끝
                m_waveTimer = 0.f;
                m_currentWaveDuration = 0.f;
                m_isActive = false;
                m_pEngineUtility->PlaySound2D("BGM_game", 0.5f);
                m_pEngineUtility->StopSound("BGM_wave");
                m_pEngineUtility->StopSound("BGM_waveBeep");
            }
        }
    }

    // 상단 wave_progress / wave_left0~6 갱신
    UpdateWaveUI(fTimeDelta);
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

void Console::StopWave()
{
    if (!m_isActive && m_waveIndex >= m_waveDurations.size())
        return;

    // 웨이브 강제 종료
    m_isActive = false;
    m_waveTimer = 0.f;
    m_currentWaveDuration = 0.f;

    // "모든 웨이브 끝" 상태로 취급되도록 인덱스를 끝까지 밀어줌
    m_waveIndex = static_cast<_uint>(m_waveDurations.size());

    // BGM 원복 및 웨이브 관련 사운드 정리
    m_pEngineUtility->StopSound("BGM_game");
    m_pEngineUtility->StopSound("BGM_wave");
    m_pEngineUtility->StopSound("BGM_waveBeep");
}

void Console::UnlockConsole()
{
    m_isLocked = false;
}

void Console::SetVisibleConsoleUI(_bool bVisible)
{
    if (bVisible == false)
    {
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_text_plate"))        p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_image_plate"))       p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_key_plate"))         p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_key_image_default")) p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_key_image_on"))      p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_image_lock"))        p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_image_unlock"))      p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_text_hacking"))      p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_text_dataload"))     p->SetVisible(false);
        return;
    }
    m_keyBlinkAcc = 0.f;
    m_keyBlinkOnState = false;

    if (UI* p = m_pEngineUtility->FindUI(L"hacking2_text_plate"))        p->SetVisible(true);
    if (UI* p = m_pEngineUtility->FindUI(L"hacking2_image_plate"))       p->SetVisible(true);
    if (UI* p = m_pEngineUtility->FindUI(L"hacking2_key_plate"))         p->SetVisible(true);
    if (UI* p = m_pEngineUtility->FindUI(L"hacking2_key_image_default")) p->SetVisible(true);
    if (UI* p = m_pEngineUtility->FindUI(L"hacking2_key_image_on"))      p->SetVisible(false);
    if (m_isLocked)
    {
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_image_lock"))        p->SetVisible(true);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_text_hacking"))      p->SetVisible(true);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_image_unlock"))      p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_text_dataload"))     p->SetVisible(false);
    }
    else
    {
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_image_lock"))        p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_text_hacking"))      p->SetVisible(false);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_image_unlock"))      p->SetVisible(true);
        if (UI* p = m_pEngineUtility->FindUI(L"hacking2_text_dataload"))     p->SetVisible(true);
    }
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

HRESULT Console::ReadyWaveUI()
{
    if (m_waveUIReady)
        return S_OK;

    m_pEngineUtility->FindUI(L"wave_text")->SetVisible(true);
    m_pEngineUtility->FindUI(L"wave_progress")->SetVisible(true);
    UI* progressbar = m_pEngineUtility->FindUI(L"wave_progressbar");
    progressbar->SetVisible(true);
    static_cast<UIImage*>(progressbar)->SetLoadingRatio(0.f);

    wchar_t name[32]{};
    for (int i = 0; i <= 6; ++i)
    {
        swprintf_s(name, L"wave_left%d", i);
        m_pEngineUtility->FindUI(name)->SetVisible(true);
    }

    m_waveUIReady = true;
    return S_OK;
}

void Console::UpdateWaveUI(_float fTimeDelta)
{
    (void)fTimeDelta;

    if (!m_waveUIReady)
        return;

    // 웨이브가 한 번이라도 시작했는지
    const bool waveStarted = (m_isActive || m_waveIndex > 0);

    // 웨이브가 전부 끝난 상태인지
    const bool waveFinished = (!m_isActive && m_waveIndex >= m_waveDurations.size());

    // 1) 웨이브 시작 전이거나, 모든 웨이브가 끝난 상태면 wave_* 전부 OFF
    if (!waveStarted || waveFinished)
    {
        // wave_left0~6 전부 끄기
        wchar_t name[32]{};
        for (int i = 0; i <= 6; ++i)
        {
            swprintf_s(name, L"wave_left%d", i);
            UI* pLeft = m_pEngineUtility->FindUI(name);
            if (!pLeft)
                continue;

            pLeft->SetVisible(false);
        }

        // 텍스트 / 바 / 배경도 끄기
        if (UI* pText = m_pEngineUtility->FindUI(L"wave_text"))        pText->SetVisible(false);
        if (UI* pBg = m_pEngineUtility->FindUI(L"wave_progress"))    pBg->SetVisible(false);
        if (UI* pBar = m_pEngineUtility->FindUI(L"wave_progressbar")) pBar->SetVisible(false);

        return;
    }

    // ===== 여기부터는 "웨이브 진행 중" 상태 =====

    // 2) 남은 웨이브 개수에 따라 wave_left0~6 중 하나만 켠다.
    _int remain = GetRemainWaveCount();
    if (remain < 0) remain = 0;
    if (remain > 6) remain = 6;

    wchar_t name[32]{};
    for (int i = 0; i <= 6; ++i)
    {
        swprintf_s(name, L"wave_left%d", i);
        UI* pLeft = m_pEngineUtility->FindUI(name);
        if (!pLeft)
            continue;

        pLeft->SetVisible(i == remain);
    }

    // 3) wave_text / wave_progress / wave_progressbar 노출 제어
    UI* pText = m_pEngineUtility->FindUI(L"wave_text");
    UI* pBg = m_pEngineUtility->FindUI(L"wave_progress");
    UI* pBar = m_pEngineUtility->FindUI(L"wave_progressbar");

    const bool showBar = m_isActive && (m_currentWaveDuration > 0.f);

    if (pText) pText->SetVisible(showBar);
    if (pBg)   pBg->SetVisible(showBar);
    if (pBar)  pBar->SetVisible(showBar);

    if (!showBar || !pBar)
        return;

    // 4) wave_progressbar를 왼쪽에서 오른쪽으로 차오르게
    if (auto pImg = dynamic_cast<UIImage*>(pBar))
    {
        _float fill = GetWaveFillRatio(); // 0 ~ 1
        if (fill < 0.f) fill = 0.f;
        if (fill > 1.f) fill = 1.f;

        pImg->SetLoadingRatio(fill);
    }
}


_float Console::GetWaveFillRatio() const
{
    if (!m_isActive)
        return 0.f;

    if (m_currentWaveDuration <= 0.f)
        return 1.f; // 0초 웨이브면 그냥 꽉 찬걸로

    // 경과 시간 = 전체 - 남은
    _float elapsed = m_currentWaveDuration - m_waveTimer;
    if (elapsed < 0.f)                 elapsed = 0.f;
    if (elapsed > m_currentWaveDuration) elapsed = m_currentWaveDuration;

    return elapsed / m_currentWaveDuration;
}

_int Console::GetRemainWaveCount() const
{
    const _int total = static_cast<_int>(m_waveDurations.size());
    if (total <= 0)
        return 0;

    _int spawned = 0;

    if (m_isActive)
    {
        // 진행 중이면 m_waveIndex 개수만큼 이미 나간 상태
        spawned = static_cast<_int>(m_waveIndex);
    }
    else
    {
        // 진행 중은 아닌데 인덱스가 끝까지 갔으면 전부 소진된 상태
        if (m_waveIndex >= m_waveDurations.size())
            spawned = total;   // 남은 웨이브 0
        else
            spawned = 0;       // 아직 시작 전
    }

    _int remain = total - spawned;
    if (remain < 0) remain = 0;
    if (remain > 6) remain = 6;

    return remain;
}

void Console::StartWaveSequence()
{
    if (m_isActive)
        return; // 이미 웨이브 진행중이면 무시

    if (m_waveIndex >= m_waveDurations.size())
        return;

    m_isActive = true;

    if (!m_waveDurations.empty())
    {
        m_waveTimer = m_waveDurations[0];
        m_currentWaveDuration = m_waveDurations[0];
    }
    else
    {
        m_waveTimer = 0.f;
        m_currentWaveDuration = 0.f;
    }

    // 첫 간격이 0이면 바로 첫 웨이브 스폰
    if (m_waveTimer <= 0.f)
    {
        m_pEngineUtility->Spawn(7);
        ++m_waveIndex;

        if (m_waveIndex < m_waveDurations.size())
        {
            m_waveTimer = m_waveDurations[m_waveIndex];
            m_currentWaveDuration = m_waveDurations[m_waveIndex];
        }
        else
        {
            m_waveTimer = 0.f;
            m_currentWaveDuration = 0.f;
            m_isActive = false;
        }
    }

    m_pEngineUtility->StopSound("BGM_game");
    m_pEngineUtility->PlaySound2D("BGM_wave", 0.5f);
    m_pEngineUtility->PlaySound2D("BGM_waveBeep", 0.7f);

    m_pEngineUtility->PlaySound2D("FBX_playerHolyshit");
    static_cast<Player*>(m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0))->SetGlobalSightScale(4.f);
}
