#include "minimapUI.h"

#include "EngineUtility.h"
#include "Transform.h"
#include "UIImage.h"
#include "Layer.h"

static std::string W2N(const std::wstring& w)
{
    return std::string(w.begin(), w.end());
}

static UIImage* CreateMinimapIcon(EngineUtility* pUtil, const std::wstring& uiName, const UI_DESC& baseDesc)
{
    if (!pUtil)
        return nullptr;

    UI_DESC desc = baseDesc;
    desc.name = W2N(uiName);
    desc.visible = true;
    desc.enable = true;

    pUtil->AddObject(SCENE::STATIC, L"UIImage", SCENE::GAMEPLAY, L"UI", &desc);
    UI* pUI = static_cast<UI*>(pUtil->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects().back());
    pUtil->AddUI(uiName, pUI);

    return static_cast<UIImage*>(pUI);
}

minimapUI::minimapUI()
    : UI{}
{
}

minimapUI::minimapUI(const minimapUI& Prototype)
    : UI{ Prototype }
{
}

HRESULT minimapUI::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;
    return S_OK;
}

HRESULT minimapUI::Initialize()
{
    UI_DESC desc{};
    desc.fRotationPerSec = 0.f;
    desc.fSpeedPerSec = 0.f;
    desc.name = "minimapUI";
    desc.visible = true;
    desc.enable = true;

    if (FAILED(__super::Initialize(&desc)))
        return E_FAIL;

    // 1) 프레임 / 배경 / 맵 이미지는 기존처럼 FindUI
    m_pFrameLeft = m_pEngineUtility->FindUI(L"minimapFrameLeft");
    m_pFrameRight = m_pEngineUtility->FindUI(L"minimapFrameRight");
    m_pFrameTop = m_pEngineUtility->FindUI(L"minimapFrameTop");
    m_pFrameBot = m_pEngineUtility->FindUI(L"minimapFrameBot");
    m_pBack = m_pEngineUtility->FindUI(L"minimapBack");
    m_pMapImage = static_cast<UIImage*>(m_pEngineUtility->FindUI(L"minimapGameScene"));

    if (!m_pFrameLeft || !m_pFrameRight || !m_pFrameTop || !m_pFrameBot ||
        !m_pBack || !m_pMapImage)
        return E_FAIL;

    static_cast<UIImage*>(m_pFrameLeft)->SetAlpha(0.5f);
    static_cast<UIImage*>(m_pFrameRight)->SetAlpha(0.5f);
    static_cast<UIImage*>(m_pFrameTop)->SetAlpha(0.5f);
    static_cast<UIImage*>(m_pFrameBot)->SetAlpha(0.5f);

    // 미니맵 중앙 계산 (백그라운드 기준)
    UI_DESC backDesc = m_pBack->GetUIDesc();
    m_viewCenter.x = backDesc.x;
    m_viewCenter.y = backDesc.y;

    // 맵 이미지 기준 위치 저장
    UI_DESC mapDesc = m_pMapImage->GetUIDesc();
    m_baseMapPos.x = mapDesc.x;
    m_baseMapPos.y = mapDesc.y;

    // 2) minimap_playerDir 생성 (중앙 고정 작은 아이콘)
    {
        UI_DESC dirDesc{};
        dirDesc.x = m_viewCenter.x - 12.f;
        dirDesc.y = m_viewCenter.y - 12.f;
        dirDesc.z = 0.01f;
        dirDesc.w = 24.f;      // 아이콘 크기
        dirDesc.h = 24.f;
        dirDesc.imagePath = L"../bin/Resources/Textures/Minimap/playerDir.png";
        dirDesc.visible = true;
        dirDesc.enable = true;

        m_pPlayerDir = CreateMinimapIcon(m_pEngineUtility,
            L"minimap_playerDir",
            dirDesc);
        if (!m_pPlayerDir)
            return E_FAIL;

        m_pPlayerDir->SetMaskingColor(_float4{ 0.f,1.f,0.f,1.f });
    }

    // 3) minimap_raider 생성 (중앙에서 커지는 링 같은 효과)
    {
        UI_DESC raiderDesc{};
        raiderDesc.x = m_viewCenter.x - 12.f;
        raiderDesc.y = m_viewCenter.y - 12.f;
        raiderDesc.z = 0.01f;
        raiderDesc.w = 24.f;
        raiderDesc.h = 24.f;
        raiderDesc.imagePath = L"../bin/Resources/Textures/Minimap/raider.png";
        raiderDesc.visible = false;  // 기본은 숨김
        raiderDesc.enable = true;

        m_pRaider = CreateMinimapIcon(m_pEngineUtility,
            L"minimap_raider",
            raiderDesc);
        if (!m_pRaider)
            return E_FAIL;

        m_raiderBaseSize.x = raiderDesc.w;
        m_raiderBaseSize.y = raiderDesc.h;

        m_pRaider->SetMaskingColor(_float4{ 1.f,1.f,1.f,1.f });
    }

    // 4) minimap_enemyPos 템플릿만 저장
    {
        UI_DESC enemyDesc{};
        enemyDesc.x = m_viewCenter.x;
        enemyDesc.y = m_viewCenter.y;
        enemyDesc.z = 0.01f;
        enemyDesc.w = 16.f;
        enemyDesc.h = 16.f;
        enemyDesc.imagePath = L"../bin/Resources/Textures/Minimap/enemyPos.png";
        enemyDesc.visible = false; // 기본은 숨김
        enemyDesc.enable = true;

        m_enemyIconDesc = enemyDesc;
        m_EnemyIcons.clear();     // 혹시 모를 쓰레기 초기화
        m_EnemyPingAlpha.clear();
    }

    // 처음엔 전체 미니맵 비활성 상태
    Show(false);

    return S_OK;
}

void minimapUI::Update(_float fTimeDelta)
{
    if (!m_isMinimapActive)
        return;
    if (!m_pPlayer || !m_pMapImage)
        return;

    UpdateMapPosition();     // 맵 이미지만 플레이어 이동량에 따라 슬라이드
    UpdatePlayerDir();       // 중심에서 플레이어 방향 표시
    UpdateRaider(fTimeDelta);// 주기적으로 나타나는 레이더 효과
    UpdateEnemyPos(fTimeDelta);        // 주변 적 위치 찍기
}

void minimapUI::Show(_bool bShow)
{
    if (m_pFrameLeft)   m_pFrameLeft->SetVisible(bShow);
    if (m_pFrameRight)  m_pFrameRight->SetVisible(bShow);
    if (m_pFrameTop)    m_pFrameTop->SetVisible(bShow);
    if (m_pFrameBot)    m_pFrameBot->SetVisible(bShow);
    if (m_pBack)        m_pBack->SetVisible(bShow);
    if (m_pMapImage)    m_pMapImage->SetVisible(bShow);

    if (m_pPlayerDir)   m_pPlayerDir->SetVisible(bShow);
    if (m_pRaider)      m_pRaider->SetVisible(bShow);
    for (auto* pIcon : m_EnemyIcons)
    {
        if (pIcon)
            pIcon->SetVisible(bShow);
    }
}

void minimapUI::StopMinimap()
{
    m_isMinimapActive = false;
    Show(false);
    // 다음에 다시 켜면 새 기준점 잡도록
    m_bOriginSet = false;

    // 레이더/파동 리셋
    m_raiderTimer = 0.f;
    m_waveActive = false;
    m_waveT = 0.f;

    // 적 아이콘들 숨기고 알파 0으로
    for (auto* pIcon : m_EnemyIcons)
    {
        if (!pIcon) continue;

        UI_DESC d = pIcon->GetUIDesc();
        d.visible = false;
        pIcon->ApplyUIDesc(d);
        pIcon->SetAlpha(0.f);
    }

    for (auto& a : m_EnemyPingAlpha)
        a = 0.f;
}

minimapUI* minimapUI::Create()
{
    minimapUI* pInstance = new minimapUI;

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : minimapUI");
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

Object* minimapUI::Clone(void* pArg)
{
    minimapUI* pInstance = new minimapUI(*this);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Cloned : minimapUI");
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void minimapUI::Free()
{
    __super::Free();
}

// --------------------------------------------------
// 1) 맵 이미지 이동 (Δpos * scale)
// --------------------------------------------------

void minimapUI::UpdateMapPosition()
{
    Transform* pTr = static_cast<Transform*>(m_pPlayer->FindComponent(TEXT("Transform")));
    if (!pTr)
        return;

    _float3 pos{};
    XMStoreFloat3(&pos, pTr->GetState(MATRIXROW_POSITION));

    // 첫 프레임에 플레이어 위치를 "원점"으로 잡는다
    if (!m_bOriginSet)
    {
        m_originWorld = pos;
        m_bOriginSet = true;
    }

    // 플레이어가 기준점에서 얼마나 이동했는지 (Δx, Δz)
    const float dx = pos.x - m_originWorld.x;
    const float dz = pos.z - m_originWorld.z;

    UI_DESC mapDesc = m_pMapImage->GetUIDesc();

    // 맵 이미지는 반대로 움직인다 (플레이어는 항상 중앙 근처에 보이도록)
    mapDesc.x = m_baseMapPos.x - dx * m_fWorldToMiniScale;
    mapDesc.y = m_baseMapPos.y + dz * m_fWorldToMiniScale;

    m_pMapImage->ApplyUIDesc(mapDesc);
}

// --------------------------------------------------
// 2) 플레이어 방향 아이콘 (minimap_playerDir)
// --------------------------------------------------

void minimapUI::UpdatePlayerDir()
{
    if (!m_pPlayerDir)
        return;

    Transform* pTr = static_cast<Transform*>(m_pPlayer->FindComponent(TEXT("Transform")));
    if (!pTr)
        return;

    _float3 look{};
    XMStoreFloat3(&look, pTr->GetState(MATRIXROW_LOOK));

    float angle = std::atan2(look.x, look.z); // 플레이어 바라보는 방향(Yaw)

    UI_DESC dirDesc = m_pPlayerDir->GetUIDesc();

    // 실제로 UI에 줄 회전값 (네가 맞춰둔 그대로)
    float rot = -XM_PIDIV2 - angle;

    // 이미지 반폭 (half width)
    float r = dirDesc.w;   // UI가 half-size 쓰는 구조니까 그대로 사용

    // ★ 회전 후에도 "왼쪽 중앙"이 m_viewCenter에 오도록
    dirDesc.x = m_viewCenter.x + r * std::cos(rot) * 0.5f;
    dirDesc.y = m_viewCenter.y - r * std::sin(rot) * 0.5f;

    m_pPlayerDir->SetRotationRad(rot);
    m_pPlayerDir->ApplyUIDesc(dirDesc);

    m_pPlayerDir->SetMaskingColor(_float4{ 0.f,1.f,0.f,1.f });
}


// --------------------------------------------------
// 3) 레이더 아이콘 (minimap_raider)
//    일정 주기마다 중심에서 나타나면서 커지는 효과
// --------------------------------------------------

void minimapUI::UpdateRaider(_float dt)
{
    if (!m_pRaider)
        return;

    m_raiderTimer += dt;

    const float cycle = m_raiderInterval + m_raiderDuration;
    if (m_raiderTimer > cycle)
    {
        m_raiderTimer -= cycle;
        m_pEngineUtility->PlaySound2D("FBX_raider");
    }

    UI_DESC desc = m_pRaider->GetUIDesc();

    if (m_raiderTimer < m_raiderInterval)
    {
        desc.visible = false;
        m_pRaider->ApplyUIDesc(desc);

        // ★ 파동 비활성
        m_waveActive = false;
        m_waveT = 0.f;
        return;
    }

    float t = (m_raiderTimer - m_raiderInterval) / m_raiderDuration; // 0 ~ 1
    if (t > 1.f) t = 1.f;

    // ★ 파동 진행도 저장
    m_waveActive = true;
    m_waveT = t;

    desc.visible = true;

    float scale = 0.5f + t * 4.5f;
    float halfW = m_raiderBaseSize.x * scale;
    float halfH = m_raiderBaseSize.y * scale;

    desc.w = halfW;
    desc.h = halfH;
    desc.x = m_viewCenter.x;
    desc.y = m_viewCenter.y;

    m_pRaider->ApplyUIDesc(desc);
    m_pRaider->SetAlpha(1 - t * t * t);
}


// --------------------------------------------------
// 4) 적 위치 아이콘 (minimap_enemyPos)
//    플레이어 주변 적 중 가장 가까운 한 개만 표시
// --------------------------------------------------

void minimapUI::UpdateEnemyPos(_float dt)
{
    // 0) 저장된 알파 먼저 감쇠 (파동과 무관하게 0.8초 동안 서서히 감소)
    if (!m_EnemyPingAlpha.empty())
    {
        for (size_t i = 0; i < m_EnemyPingAlpha.size(); ++i)
        {
            if (m_EnemyPingAlpha[i] > 0.f)
            {
                m_EnemyPingAlpha[i] -= dt / m_enemyFadeDuration; // 0.8초 동안 1 → 0
                if (m_EnemyPingAlpha[i] < 0.f)
                    m_EnemyPingAlpha[i] = 0.f;
            }
        }
    }

    // 플레이어가 없으면 전부 숨김 + 여기서 끝
    if (!m_pPlayer)
    {
        for (auto* pIcon : m_EnemyIcons)
        {
            if (!pIcon) continue;
            UI_DESC d = pIcon->GetUIDesc();
            d.visible = false;
            pIcon->ApplyUIDesc(d);
        }
        return;
    }

    Transform* pPlayerTr = static_cast<Transform*>(m_pPlayer->FindComponent(TEXT("Transform")));
    if (!pPlayerTr)
    {
        for (auto* pIcon : m_EnemyIcons)
        {
            if (!pIcon) continue;
            UI_DESC d = pIcon->GetUIDesc();
            d.visible = false;
            pIcon->ApplyUIDesc(d);
        }
        return;
    }

    _float3 playerPos{};
    XMStoreFloat3(&playerPos, pPlayerTr->GetState(MATRIXROW_POSITION));

    const float maxDistSq = m_enemyMaxWorldDist * m_enemyMaxWorldDist;

    // 1) Drone / Worm / Shieldbug 전부 모으기
    std::vector<Object*> monsters;
    monsters.reserve(64);

    auto CollectFromLayer = [&](const wchar_t* layerName)
        {
            Layer* pLayer = m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, layerName);
            if (!pLayer)
                return;

            const list<Object*>& objs = pLayer->GetAllObjects();
            monsters.insert(monsters.end(), objs.begin(), objs.end());
        };

    CollectFromLayer(L"Drone");
    CollectFromLayer(L"Worm");
    CollectFromLayer(L"Shieldbug");

    if (monsters.empty())
    {
        for (auto* pIcon : m_EnemyIcons)
        {
            if (!pIcon) continue;
            UI_DESC d = pIcon->GetUIDesc();
            d.visible = false;
            pIcon->ApplyUIDesc(d);
        }
        return;
    }

    _uint activeCount = 0;

    for (Object* pEnemy : monsters)
    {
        if (!pEnemy) continue;
        if (pEnemy->IsDead()) continue;

        Transform* pTr = static_cast<Transform*>(pEnemy->FindComponent(TEXT("Transform")));
        if (!pTr) continue;

        _float3 epos{};
        XMStoreFloat3(&epos, pTr->GetState(MATRIXROW_POSITION));

        float dx = epos.x - playerPos.x;
        float dz = epos.z - playerPos.z;
        float distSq = dx * dx + dz * dz;

        if (distSq > maxDistSq)
            continue;

        // 아이콘 풀에서 하나 가져오거나, 없으면 새로 생성
        Engine::UIImage* pIcon = nullptr;

        if (activeCount < m_EnemyIcons.size())
        {
            pIcon = m_EnemyIcons[activeCount];
        }
        else
        {
            UI_DESC iconDesc = m_enemyIconDesc;
            iconDesc.x = m_viewCenter.x;
            iconDesc.y = m_viewCenter.y;

            std::wstring uiName = L"minimap_enemyPos_" + std::to_wstring(activeCount);

            pIcon = CreateMinimapIcon(m_pEngineUtility, uiName, iconDesc);
            if (!pIcon)
                break;

            pIcon->SetMaskingColorGradient(_float4{ 1.f, 0.f, 0.f, 1.f });

            m_EnemyIcons.push_back(pIcon);
            m_EnemyPingAlpha.push_back(0.f); // ★ 새 아이콘은 알파 0
        }

        if (!pIcon)
            continue;

        // 아이콘 위치 갱신
        UI_DESC desc = pIcon->GetUIDesc();

        float offsetX = dx * m_fWorldToMiniScale;
        float offsetY = dz * m_fWorldToMiniScale;

        desc.visible = true;
        desc.x = m_viewCenter.x + offsetX;
        desc.y = m_viewCenter.y - offsetY;

        pIcon->ApplyUIDesc(desc);

        // ★ 파동이 활성 상태일 때 "파동이 이 적을 스쳤는지" 확인
        if (m_waveActive)
        {
            float dist = sqrtf(distSq);

            float distNorm = 0.f;
            if (m_enemyMaxWorldDist > 0.f)
                distNorm = dist / m_enemyMaxWorldDist;
            distNorm = std::clamp(distNorm, 0.f, 1.f);

            float waveR = m_waveT;

            const float bandWidth = 0.25f;

            float diff = fabsf(distNorm - waveR);
            float band = 1.0f - std::clamp(diff / bandWidth, 0.f, 1.f);

            if (band > 0.f)
            {
                // 파동이 닿은 프레임: 저장 알파를 확 올려줌 (band 비율만큼)
                float newAlpha = band; // 0~1
                if (activeCount < m_EnemyPingAlpha.size())
                {
                    // 이미 남아 있는 알파보다 크면 갱신
                    if (newAlpha > m_EnemyPingAlpha[activeCount])
                        m_EnemyPingAlpha[activeCount] = newAlpha;
                }
            }
        }

        // ★ 저장된 알파를 실제 아이콘에 적용
        if (activeCount < m_EnemyPingAlpha.size())
            pIcon->SetAlpha(m_EnemyPingAlpha[activeCount]);
        else
            pIcon->SetAlpha(0.f);

        ++activeCount;
    }

    // 이번 프레임에 안 쓴 나머지 아이콘들은 숨기기
    for (_uint i = activeCount; i < m_EnemyIcons.size(); ++i)
    {
        Engine::UIImage* pIcon = m_EnemyIcons[i];
        if (!pIcon) continue;

        UI_DESC d = pIcon->GetUIDesc();
        d.visible = false;
        pIcon->ApplyUIDesc(d);

        // 안 쓰는 아이콘은 알파도 0으로
        if (i < m_EnemyPingAlpha.size())
            m_EnemyPingAlpha[i] = 0.f;
    }
}
