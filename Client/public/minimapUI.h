#pragma once

#include "Client_Defines.h"
#include "UI.h"

#include <vector>

NS_BEGIN(Engine)
class UIImage;
NS_END

NS_BEGIN(Client)

class minimapUI : public UI
{
private:
    minimapUI();
    minimapUI(const minimapUI& Prototype);
    virtual ~minimapUI() = default;

public:
    // Prototype / Instance 초기화
    virtual HRESULT InitializePrototype() override;
    HRESULT Initialize();
    virtual void Update(_float fTimeDelta) override;

    // 연결용
    void SetPlayer(Object* pPlayer) { m_pPlayer = pPlayer; }
    // 월드 1.0 유닛 이동당 미니맵에서 몇 픽셀 움직일지
    void SetScale(_float s) { m_fWorldToMiniScale = s; }

    void Show(_bool bShow);

    void StopMinimap();

    static minimapUI* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void     Free() override;

private:
    void UpdateMapPosition();        // 맵 이미지 이동
    void UpdatePlayerDir();          // 플레이어 방향 아이콘
    void UpdateRaider(_float dt);    // 주기적 레이더 효과
    void UpdateEnemyPos(_float dt);           // 주변 적 위치 아이콘

private:
    // 대상
    Object* m_pPlayer = nullptr;

    // UI 요소
    UI* m_pFrameLeft = nullptr;
    UI* m_pFrameRight = nullptr;
    UI* m_pFrameTop = nullptr;
    UI* m_pFrameBot = nullptr;
    UI* m_pBack = nullptr;
    Engine::UIImage* m_pMapImage = nullptr; // minimapGameScene

    Engine::UIImage* m_pPlayerDir = nullptr; // minimap_playerDir
    Engine::UIImage* m_pRaider = nullptr; // minimap_raider

    // 맵 이미지 기준 위치 (플레이어 원점일 때)
    _float2          m_baseMapPos = _float2(0.f, 0.f);
    _float3          m_originWorld = _float3(0.f, 0.f, 0.f);
    _bool            m_bOriginSet = false;

    // 미니맵 중심 (프레임/백그라운드 기준)
    _float2          m_viewCenter = _float2(0.f, 0.f);

    // 월드 → 미니맵 이동 스케일
    _float           m_fWorldToMiniScale = 1.f;

    // 레이더(minimap_raider) 애니메이션용
    _float           m_raiderTimer = 0.f;
    _float           m_raiderInterval = 1.0f;  // N초마다 한 번
    _float           m_raiderDuration = 0.45f;  // 커지는 시간
    _float2          m_raiderBaseSize = _float2(0.f, 0.f);
    _bool            m_waveActive = false;
    _float           m_waveT = 0.f;

    _float           m_enemyMaxWorldDist = 23.f; // 이 거리 이내만 표시
    UI_DESC                  m_enemyIconDesc{};      // 아이콘 템플릿 (크기/텍스처 등)
    std::vector<Engine::UIImage*> m_EnemyIcons;
    std::vector<_float> m_EnemyPingAlpha;
    _float           m_enemyFadeDuration = 0.8f;

    _bool            m_isMinimapActive = true;
};

NS_END
