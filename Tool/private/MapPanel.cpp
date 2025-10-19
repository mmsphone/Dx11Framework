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
    ImGui::Text("Mouse LB To Place Marker");
    ImGui::Text("Mouse RB To Add Navigation Cell");
    static _float4 pickPos{};
    TestTerrain* pTerrain = dynamic_cast<TestTerrain*>(m_pTerrain);
    if (pTerrain != nullptr)
        pickPos = pTerrain->GetBrushPos();
    ImGui::Text("Recent Pick Position : %f %f %f", pickPos.x, pickPos.y, pickPos.z);
    ImGui::Text("\n");

    if (ImGui::Button("Remove Recent Navigation Cell"))
    {
        Navigation* pNavigation = dynamic_cast<Navigation*>(m_pTerrain->FindComponent(TEXT("Navigation")));
        if (!pNavigation)
            return;
        pNavigation->RemoveRecentCell();
    }
    if (ImGui::Button("Save Navigation Cells"))
    {
        Navigation* pNavigation = dynamic_cast<Navigation*>(m_pTerrain->FindComponent(TEXT("Navigation")));
        if (!pNavigation)
            return;
        pNavigation->SaveCells(TEXT("../bin/data/Navigation.dat"));
    } 
    if (ImGui::Button("Place FieldObject on Marker"))
    {
        if (m_pTerrain == nullptr)
            return;
        
        _uint iIndex = m_pEngineUtility->GetLayerSize(SCENE::MAP, TEXT("FieldObject"));
        m_pEngineUtility->AddObject(SCENE::MAP, TEXT("Prototype_GameObject_FieldObject"), SCENE::MAP, TEXT("FieldObject"));
        Object* pObject = m_pEngineUtility->FindObject(SCENE::MAP, TEXT("FieldObject"), iIndex);
        if (pObject == nullptr)
            return;

        Transform* pTransform = dynamic_cast<Transform*>(pObject->FindComponent(TEXT("Transform")));
        if (pTransform == nullptr)
            return;
        
        TestTerrain* pTerrain = dynamic_cast<TestTerrain*>(m_pTerrain);
        if (pTerrain == nullptr)
            return;
        
        _float4 fPos = pTerrain->GetBrushPos();
        _vector vPos = XMLoadFloat4(&fPos);

        Navigation* pNavigation = dynamic_cast<Navigation*>(pTerrain->FindComponent(TEXT("Navigation")));
        if (pNavigation == nullptr)
            return;
        _bool isInCell = pNavigation->IsInCell(vPos);
        if(isInCell == true)
            pNavigation->SetHeightOnCell(vPos, &vPos);
        pTransform->SetState(POSITION, vPos);
    }
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