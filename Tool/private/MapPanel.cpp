#include "MapPanel.h"

#include "EngineUtility.h"

#include "ModelScene.h"
#include "TestTerrain.h"
#include "IEHelper.h"
#include "Cell.h"
#include "ObjectPanel.h"

MapPanel::MapPanel(const string& PanelName, bool open)
    :Panel{ PanelName, open }
{
}

HRESULT MapPanel::Initialize()
{
    m_PanelPosition = _float2(0.f, 0.f);
    m_PanelSize = _float2(400.f, 300.f);

    return S_OK;
}

void MapPanel::OnRender()
{
    //씬 전환
    if (ImGui::Button("ModelScene"))
    {
        m_pEngineUtility->SetPathToBin();
        m_pEngineUtility->ChangeScene(SCENE::MODEL, ModelScene::Create(SCENE::MODEL));
        
        m_pEngineUtility->SetPanelOpen("MapPanel", false);
        m_pEngineUtility->SetPanelOpen("MapCamPanel", false);
        m_pEngineUtility->SetPanelOpen("AssetPanel", false);
        if (m_pSelectedObject != nullptr)
            m_pEngineUtility->SetPanelOpen("ObjectPanel", false);

    }
    ImGui::Separator();
    
    //오브젝트, 지형, 그리드 피킹
    ImGui::Text("Click To Place Marker");
    static PICK_RESULT s_pick{};

    if (m_CellMode == false && m_pEngineUtility->IsMousePressed(LB))
    {
        PICK_RESULT pick = m_pEngineUtility->Pick();
        if (pick.hit == true)
        {
            switch (pick.pickType)
            {
            case PICK_OBJECT:
                if (pick.pHitObject != m_pSelectedObject)
                {
                    m_pSelectedObject = pick.pHitObject;

                    if (m_pObjectPanel)
                    {
                        m_pEngineUtility->RemovePanel(m_pObjectPanel->GetPanelName());
                        SafeRelease(m_pObjectPanel);
                        m_pObjectPanel = nullptr;
                    }

                    std::string objPanelName = "ObjectPanel";
                    m_pObjectPanel = ObjectPanel::Create(objPanelName, true, m_pSelectedObject);
                    m_pEngineUtility->AddPanel(objPanelName, m_pObjectPanel);
                }
                break;
            case PICK_TERRAIN:
            {
                s_pick = pick;
                TestTerrain* pTerrain = dynamic_cast<TestTerrain*>(pick.pHitObject);
                if (pTerrain != nullptr)
                {
                    _float4 vPos = { pick.hitPos.x, pick.hitPos.y, pick.hitPos.z, 1.f };
                    pTerrain->SetBrushPos(vPos);
                }
                break;
            }
            case PICK_GRID:
                s_pick = pick;
                m_pEngineUtility->SetMarkerPosition(pick.hitPos);
                break;
            default:
                break;
            }
        }
    }

    if (s_pick.hit)
    {
        const char* pickTypeName =
            (s_pick.pickType == PICK_TERRAIN) ? "Terrain" :
            (s_pick.pickType == PICK_GRID) ? "Grid" :
            (s_pick.pickType == PICK_OBJECT) ? "Object" : "Unknown";

        ImGui::Text("Marker Type: %s", pickTypeName);
        ImGui::Text("Marker Pos: %.2f, %.2f, %.2f",
            s_pick.hitPos.x, s_pick.hitPos.y, s_pick.hitPos.z);
    }
    else
        ImGui::TextDisabled("No marker placed yet");
    ImGui::Separator();

    //네비 셀 추가
    if (ImGui::Checkbox("Cell Mode", &m_CellMode))
        ImGui::Text(m_CellMode ? "Cell Mode: ON" : "Cell Mode: OFF");
    
    if (m_CellMode && m_pEngineUtility->IsMousePressed(MOUSEKEYSTATE::LB))
    {
        PICK_RESULT pick = m_pEngineUtility->Pick();
        if (pick.hit && (pick.pickType == PICK_TERRAIN || pick.pickType == PICK_GRID))
        {
            m_pEngineUtility->AddTempPoint(pick.hitPos);
        }
    }
    //최신 셀 제거
    if (ImGui::Button("Remove Recent Navigation Cell"))
    {
        m_pEngineUtility->RemoveRecentCell();
    }
    //네비게이션 데이터 경로
    ImGui::Text("Navigation Data Path:");
    if (!m_NavigationDataPath.empty())
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.f), "%s", m_NavigationDataPath.c_str());
    else
        ImGui::TextDisabled("None");
    //네비 셀 로드
    if (ImGui::Button("Load Navigation Cells"))
    {
        m_pEngineUtility->LoadCells(m_NavigationDataPath.c_str());
    }
    ImGui::SameLine();
    //네비 셀 저장
    if (ImGui::Button("Save Navigation Cells"))
    {
        m_pEngineUtility->SaveCells(m_NavigationDataPath.c_str());
    } 
    ImGui::Separator();
    //모델 데이터 경로
    ImGui::Text("Model Data Path:");
    if (!m_SelectedModelDataPath.empty())
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.f), "%s", m_SelectedModelDataPath.c_str());
    else
        ImGui::TextDisabled("None");
    ImGui::Separator();
    //필드 오브젝트 추가
    if (ImGui::Button("Place FieldObject on Marker"))
    {
        if (s_pick.hit == false)
        {
            MessageBoxA(nullptr, "No valid marker or pick point found.", "Placement Error", MB_OK);
            return;
        }

        _uint iIndex = m_pEngineUtility->GetLayerSize(SCENE::MAP, TEXT("FieldObject"));
        m_pEngineUtility->AddObject(SCENE::MAP, TEXT("FieldObject"),
            SCENE::MAP, TEXT("FieldObject"));
        Object* pObject = m_pEngineUtility->FindObject(SCENE::MAP, TEXT("FieldObject"), iIndex);
        if (!pObject) 
            return;

        if (!m_SelectedModelDataPath.empty())
        {
            Model* pModel = dynamic_cast<Model*>(pObject->FindComponent(TEXT("Model")));
            if (pModel)
            {
                ModelData* pModelData = pModel->LoadNoAssimpModel(m_SelectedModelDataPath.c_str());
                if (pModelData)
                    pModel->SetModelData(pModelData);
            }
        }

        Transform* pTransform = dynamic_cast<Transform*>(pObject->FindComponent(TEXT("Transform")));
        if (!pTransform) 
            return;

        _vector vPos = XMVectorSet(s_pick.hitPos.x, s_pick.hitPos.y, s_pick.hitPos.z, 1.f);

        if (m_pEngineUtility->IsInCell(vPos))
        {
            _vector adjusted = {};
            m_pEngineUtility->SetHeightOnCell(vPos, &adjusted);
            vPos = adjusted;
        }
        pTransform->SetState(POSITION, vPos);
    }
}

void MapPanel::SetSelectedBinPath(const string& path)
{
    m_SelectedModelDataPath = path;
}

void MapPanel::SetNavigationDataPath(const string& path)
{
    m_NavigationDataPath = path;
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
}