#pragma once

#include "Tool_Defines.h"
#include "Panel.h"

NS_BEGIN(Engine)
class Light;
class ShadowLight;
NS_END

NS_BEGIN(Tool)

class LightPanel final : public Panel
{
	LightPanel(const std::string& panelName, bool open);
	virtual ~LightPanel() = default;

public:
    HRESULT Initialize();
    virtual void OnRender() override;

    static LightPanel* Create(const string& PanelName, bool open = true);
    virtual void		Free() override;

private:
    void SyncFromSelectedLight();
    void ApplyToSelectedLight();
    void DeleteSelectedLight();
    void SyncFromSelectedShadowLight();
    void ApplyToSelectedShadowLight();
    void DeleteSelectedShadowLight();

private:
    _int          m_SelectedLightIndex = -1;
    class Light* m_pSelectedLight = nullptr;

    // UI 편집용 버퍼
    _int     m_LightType = 1; // 0=Directional, 1=Point, 2=Spot
    XMFLOAT4 m_Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.f);
    XMFLOAT4 m_Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
    XMFLOAT4 m_Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.f);

    XMFLOAT4 m_Direction = XMFLOAT4(0.f, -1.f, 0.f, 0.f);
    XMFLOAT4 m_Position = XMFLOAT4(0.f, 5.f, 0.f, 1.f);

    float m_Range = 15.f;
    float m_InnerDeg = 20.f;
    float m_OuterDeg = 40.f;

    // --- ★ ShadowLight 편집용 선택/버퍼 ---
    _int                m_SelectedShadowIndex = -1;
    class ShadowLight* m_pSelectedShadow = nullptr;

    XMFLOAT3 m_EditShadowEye = XMFLOAT3(0.f, 6.f, 0.f);
    XMFLOAT3 m_EditShadowAt = XMFLOAT3(0.f, 5.f, -1.f);
    float    m_EditShadowFovy = 75.f;   // deg
    float    m_EditShadowNear = 0.1f;
    float    m_EditShadowFar = 50.f;

    // 섀도우 라이트 같이 추가할지 여부 (새 라이트 생성용)
    bool m_CastShadow = true;

    // 새 라이트 생성용 쉐도우 데이터
    XMFLOAT3 m_ShadowEye = XMFLOAT3(0.f, 6.f, 0.f);
    XMFLOAT3 m_ShadowAt = XMFLOAT3(0.f, 5.f, -1.f);
    float    m_ShadowFovy = 75.f;
    float    m_ShadowNear = 0.1f;
    float    m_ShadowFar = 50.f;
};

NS_END