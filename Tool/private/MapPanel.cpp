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
    static _bool CellMode = false;
    //씬 전환
    if (ImGui::Button("ModelScene"))
    {
        m_pEngineUtility->SetPathToBin();

        m_pEngineUtility->ChangeScene(SCENE::MODEL, ModelScene::Create(SCENE::MODEL));
        m_pEngineUtility->SetPanelOpen("MapPanel", false);
        m_pEngineUtility->SetPanelOpen("MapCamPanel", false);
        if (m_pSelectedObject != nullptr)
            m_pEngineUtility->SetPanelOpen("ObjectPanel", false);
    }
    ImGui::Text("\n");

    //지형 마커
    ImGui::Text("Mouse LB Click To Place Marker");
    static _float4 markerPos{};
    TestTerrain* pTerrain = dynamic_cast<TestTerrain*>(m_pTerrain);
    if (pTerrain != nullptr)
        markerPos = pTerrain->GetBrushPos();
    ImGui::Text("Recent Pick Position : %f %f %f", markerPos.x, markerPos.y, markerPos.z);
    ImGui::Text("\n");

    //네비 셀 추가
    if (ImGui::Checkbox("Cell Mode", &CellMode))
        ImGui::Text(CellMode ? "Cell Mode: ON" : "Cell Mode: OFF");
    static bool bPrevRB = false;
    bool bCurrRB = m_pEngineUtility->GetMouseState(MOUSEKEYSTATE::RB);
    if (CellMode && bCurrRB && !bPrevRB)
    {
        PICK_RESULT pick = m_pEngineUtility->Pick();
        if (pick.hit)
        {
            Terrain* pTerrain = dynamic_cast<Terrain*>(pick.pHitObject);
            if (!pTerrain)
                return;

            Navigation* pNav = dynamic_cast<Navigation*>(pTerrain->FindComponent(TEXT("Navigation")));
            if (!pNav)
                return;

            static vector<_float3> vTempPoints{};
            _float3 hit = pick.hitPos;

            if (pNav->GetCells().empty())
            {
                if (vTempPoints.size() < 3)
                    vTempPoints.push_back(hit);

                if (vTempPoints.size() == 3)
                {
                    SortPointsClockWise(vTempPoints);
                    pNav->AddCell(&vTempPoints[0]);
                    vTempPoints.clear();
                }
            }
            else
            {
                const auto& cells = pNav->GetCells();
                Cell* last = cells.back();

                _float3 pts[3];
                XMStoreFloat3(&pts[0], last->GetPoint(POINTTYPE::A));
                XMStoreFloat3(&pts[1], last->GetPoint(POINTTYPE::B));
                XMStoreFloat3(&pts[2], last->GetPoint(POINTTYPE::C));

                pair<int, int> closestPair = { 0,1 };
                float minSum = FLT_MAX;

                for (int i = 0; i < 3; ++i)
                    for (int j = i + 1; j < 3; ++j)
                    {
                        float sum = XMVectorGetX(XMVector3Length(XMLoadFloat3(&pts[i]) - XMLoadFloat3(&hit))) +
                            XMVectorGetX(XMVector3Length(XMLoadFloat3(&pts[j]) - XMLoadFloat3(&hit)));
                        if (sum < minSum)
                        {
                            minSum = sum;
                            closestPair = { i,j };
                        }
                    }

                _float3 newTri[3] = { pts[closestPair.first], pts[closestPair.second], hit };
                pNav->AddCell(&newTri[0]);
            }
        }
    }
    bPrevRB = bCurrRB;

    //최신 셀 제거
    if (ImGui::Button("Remove Recent Navigation Cell"))
    {
        Navigation* pNavigation = dynamic_cast<Navigation*>(m_pTerrain->FindComponent(TEXT("Navigation")));
        if (!pNavigation)
            return;
        pNavigation->RemoveRecentCell();
    }

    //네비 셀 저장
    if (ImGui::Button("Save Navigation Cells"))
    {
        Navigation* pNavigation = dynamic_cast<Navigation*>(m_pTerrain->FindComponent(TEXT("Navigation")));
        if (!pNavigation)
            return;
        pNavigation->SaveCells(TEXT("../bin/data/Navigation.dat"));
    } 

    //바이너리 모델 선택
    static std::string g_SelectedBinPath{};
    ImGui::Separator();
    ImGui::Text("--Selected BIN--");
    ImGui::Text("%s", g_SelectedBinPath.empty() ? "None" : g_SelectedBinPath.c_str());
    if (ImGui::Button("Select BIN"))
    {
        char path[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFilter = "Binary Files\0*.bin\0All Files\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST;
        if (GetOpenFileNameA(&ofn))
            g_SelectedBinPath = path;
    }

    //필드 오브젝트 추가
    if (ImGui::Button("Place FieldObject on Marker"))
    {
        if (!m_pTerrain) 
            return;

        _uint iIndex = m_pEngineUtility->GetLayerSize(SCENE::MAP, TEXT("FieldObject"));
        m_pEngineUtility->AddObject(SCENE::MAP, TEXT("FieldObject"),
            SCENE::MAP, TEXT("FieldObject"));
        Object* pObject = m_pEngineUtility->FindObject(SCENE::MAP, TEXT("FieldObject"), iIndex);
        if (!pObject) return;

        if (!g_SelectedBinPath.empty())
        {
            Model* pModel = dynamic_cast<Model*>(pObject->FindComponent(TEXT("Model")));
            if (pModel)
            {
                ModelData* pModelData = pModel->LoadNoAssimpModel(g_SelectedBinPath.c_str());
                if (pModelData)
                    pModel->SetModelData(pModelData);
                else
                    MessageBoxA(nullptr, "Failed to load binary model", "Import Error", MB_OK | MB_ICONERROR);
            }
        }

        Transform* pTransform = dynamic_cast<Transform*>(pObject->FindComponent(TEXT("Transform")));
        if (!pTransform) 
            return;

        _vector vPos{};
        TestTerrain* pTerrain = dynamic_cast<TestTerrain*>(m_pTerrain);
        if (pTerrain)
        {
            _float4 fPos = pTerrain->GetBrushPos();
            _vector vPos = XMLoadFloat4(&fPos);

            Navigation* pNavigation = dynamic_cast<Navigation*>(pTerrain->FindComponent(TEXT("Navigation")));
            if (pNavigation && pNavigation->IsInCell(vPos))
                pNavigation->SetHeightOnCell(vPos, &vPos);
        }
        else
        {
            m_pEngineUtility->GetMarkerPosition();
        }
        pTransform->SetState(POSITION, vPos);
    }

    //오브젝트 피킹
    if (!CellMode && m_pEngineUtility->GetMouseState(MOUSEKEYSTATE::LB))
    {
        PICK_RESULT pick = m_pEngineUtility->Pick();
        if (pick.hit && !dynamic_cast<Terrain*>(pick.pHitObject))
        {
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
        }
    }
}

void MapPanel::SortPointsClockWise(vector<_float3>& points)
{
    if (points.size() != 3) return;

    _float3 center = {
        (points[0].x + points[1].x + points[2].x) / 3.0f,
        (points[0].y + points[1].y + points[2].y) / 3.0f,
        (points[0].z + points[1].z + points[2].z) / 3.0f
    };

    std::sort(points.begin(), points.end(),
        [&](const _float3& a, const _float3& b)
        {
            float angleA = atan2f(a.z - center.z, a.x - center.x);
            float angleB = atan2f(b.z - center.z, b.x - center.x);
            return angleA > angleB; // 시계 방향
        });
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