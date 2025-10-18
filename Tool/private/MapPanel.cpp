#include "MapPanel.h"

#include "Tool_Defines.h"

#include "EngineUtility.h"
#include "ModelScene.h"

#include "TestTerrain.h"

MapPanel::MapPanel(const string& PanelName, bool open)
    :Panel{ PanelName, open }
{
}

HRESULT MapPanel::Initialize()
{
    m_pTerrain = m_pEngineUtility->FindObject(SCENE::MAP, TEXT("Terrain"), 0);
    if (m_pTerrain == nullptr) 
        return E_FAIL;
    SafeAddRef(m_pTerrain);

    m_PanelPosition = _float2(0.f, 0.f);
    m_PanelSize = _float2(400.f, 600.f);

    return S_OK;
}

void MapPanel::OnRender()
{
    if (ImGui::Button("ModelScene"))
    {
        m_pEngineUtility->ChangeScene(SCENE::MODEL, ModelScene::Create(SCENE::MODEL));
        m_pEngineUtility->SetPanelOpen(GetPanelName(), false);
    }
    ImGui::Text("\n");
    auto camPos = m_pEngineUtility->GetCamPosition();
    ImGui::Text("Current Cam Position : %f %f %f", camPos->x, camPos->y,camPos->z);
    ImGui::Text("\n");
    ImGui::Text("Pick Terrain With LeftButton To Place Marking Texture");
    ImGui::Text("Pick Terrain With RightButton To Add Navigation Cell");
    static _float4 pickPos{};
    TestTerrain* pTerrain = dynamic_cast<TestTerrain*>(m_pTerrain);
    if (pTerrain != nullptr)
        pickPos = pTerrain->GetBrushPos();
    ImGui::Text("Recent Pick Position : %f %f %f", pickPos.x, pickPos.y, pickPos.z);
    ImGui::Text("\n");
    ImGui::Text("Press R To Remove Recent Navigation Cell");
    ImGui::Text("Press L To Save Navigation Cells");
    ImGui::Text("Press O To Place FieldObject");
}


MapPanel* MapPanel::Create(const string& PanelName, bool open)
{
    MapPanel* pInstance = new MapPanel(PanelName, open);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : MapPanel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void MapPanel::Free()
{
    __super::Free();
    SafeRelease(m_pTerrain);
}