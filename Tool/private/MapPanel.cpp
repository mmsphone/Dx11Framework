#include "MapPanel.h"

#include "EngineUtility.h"

#include "ModelScene.h"
#include "TestTerrain.h"
#include "IEHelper.h"
#include "Cell.h"
#include "ObjectPanel.h"
#include "Layer.h"

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
            case PICK_PIXEL:
                s_pick = pick;
                m_pEngineUtility->SetMarkerPosition(pick.hitPos);
                break;
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
            (s_pick.pickType == PICK_OBJECT) ? "Object" :
            (s_pick.pickType == PICK_PIXEL) ? "Pixel" : "Unknown";

        ImGui::Text("Marker Type: %s", pickTypeName);
        ImGui::Text("Marker Pos: %.2f, %.2f, %.2f",
            s_pick.hitPos.x, s_pick.hitPos.y, s_pick.hitPos.z);
    }
    else
    {
        ImGui::TextDisabled("No marker placed yet");
    }
    ImGui::Separator();

    // --- 네비 모드 ---
    if (ImGui::Checkbox("Cell Mode", &m_CellMode))
        ImGui::Text(m_CellMode ? "Cell Mode: ON" : "Cell Mode: OFF");

    // 상태 표시
    {
        const int cellCount = static_cast<int>(m_pEngineUtility->GetCells().size());
        ImGui::TextDisabled("Selected Cell: %d (0 ~ %d)", m_SelectedCellIndex, cellCount > 0 ? cellCount - 1 : 0);
    }

    // 용접 허용치
    static float s_WeldEps = 0.01f;
    ImGui::SetNextItemWidth(100.f);
    ImGui::InputFloat("WeldEps", &s_WeldEps, 0.0f, 0.0f, "%.4f");

    // ---- 핵심 분기: CellMode 중 좌클릭 처리 ----
    if (m_CellMode && m_pEngineUtility->IsMousePressed(MOUSEKEYSTATE::LB))
    {
        bool added = false; // 삼각형 추가 여부
        PICK_RESULT pick = m_pEngineUtility->Pick();
        if (pick.hit && (pick.pickType == PICK_TERRAIN || pick.pickType == PICK_GRID || pick.pickType == PICK_PIXEL))
        {
            _int hitCell = -1;
            const bool inNav = m_pEngineUtility->IsInCell(XMLoadFloat3(&pick.hitPos), &hitCell);

            if (inNav)
            {
                if (m_SelectedCellIndex == -1)
                {
                    // (1) 아무 것도 선택 안 된 상태에서 네비 내부 클릭 -> 셀 선택
                    m_SelectedCellIndex = hitCell;
                    m_pEngineUtility->SetMarkerPosition(pick.hitPos);
                }
                else
                {
                    if (m_SelectedCellIndex != hitCell)
                    {
                        // (2) 다른 셀을 클릭 -> 두 셀이 한 점을 공유한다면 "정점공유 삼각형" 추가
                        if (!m_pEngineUtility->Edit_AddTriangleAtSharedVertex(m_SelectedCellIndex, hitCell, s_WeldEps))
                        {
                            ImGui::OpenPopup("SharedVertexError");
                        }
                        else
                        {
                            m_pEngineUtility->SetMarkerPosition(pick.hitPos);
                            m_pEngineUtility->ClearTempPoints();
                            added = true;
                        }
                    }
                    else
                    {
                        // (3) 같은 셀을 다시 클릭 -> 선택 해제
                        m_SelectedCellIndex = -1;
                    }
                }
            }
            else
            {
                if (m_SelectedCellIndex != -1)
                {
                    // (4) 셀을 선택한 상태에서 네비 바깥을 클릭 -> 선택된 셀을 기준으로 "엣지 확장"
                    m_pEngineUtility->ClearTempPoints();
                    if (!m_pEngineUtility->Edit_AddTriangleOnEdge(m_SelectedCellIndex, XMLoadFloat3(&pick.hitPos), s_WeldEps))
                    {
                        ImGui::OpenPopup("EdgeExtendError");
                    }
                    else
                    {
                        m_pEngineUtility->SetMarkerPosition(pick.hitPos);
                        added = true;
                    }
                }
                else
                {
                    // (5) 선택된 셀이 없는 상태에서 네비 바깥 클릭 -> 평소처럼 TempPoint 추가
                    m_pEngineUtility->AddTempPoint(pick.hitPos);
                }
            }
        }

        // 팝업
        if (ImGui::BeginPopup("SharedVertexError"))
        {
            ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "Failed to add triangle at shared vertex.");
            ImGui::TextDisabled("Pick two cells that share exactly one vertex.");
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopup("EdgeExtendError"))
        {
            ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "Failed to extend triangle on edge.");
            ImGui::TextDisabled("Pick any point outside navigation after selecting a cell.");
            ImGui::EndPopup();
        }

        // 삼각형이 추가되었다면 선택 해제
        if (added)
            m_SelectedCellIndex = -1;
    }

    // 최신 셀 제거
    if (ImGui::Button("Remove Recent Navigation Cell"))
    {
        m_pEngineUtility->RemoveRecentCell();
        // 제거 후 선택 인덱스 무효화
        m_SelectedCellIndex = -1;
    }

    // --- 네비게이션 데이터: 파일 대화상자로 로드/저장 ---
    if (ImGui::Button("Load Navigation Cells"))
    {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Navigation Data (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileNameA(&ofn))
            m_pEngineUtility->LoadCells(szFile);
        m_SelectedCellIndex = -1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Navigation Cells"))
    {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Navigation Data (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

        if (GetSaveFileNameA(&ofn))
            m_pEngineUtility->SaveCells(szFile);
    }
    ImGui::Separator();

    // 모델 데이터 경로
    ImGui::Text("Model Data Path:");
    if (!m_SelectedModelDataPath.empty())
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.f), "%s", m_SelectedModelDataPath.c_str());
    else
        ImGui::TextDisabled("None");
    ImGui::Separator();

    // 필드 오브젝트 추가
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
                ModelData* pModelData = m_pEngineUtility->LoadNoAssimpModel(m_SelectedModelDataPath.c_str());
                if (pModelData)
                {
                    pModel->SetModelData(pModelData);
                    pModel->SetBinPath(m_SelectedModelDataPath);
                }
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
    ImGui::Separator();

    //맵 데이터 로드/저장
    if (ImGui::Button("Load Map Data"))
    {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Map Data (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        auto DumpBasis = [&](Transform* t)
            {
                XMVECTOR R = XMVector3Normalize(t->GetState(RIGHT));
                XMVECTOR U = XMVector3Normalize(t->GetState(UP));
                XMVECTOR L = XMVector3Normalize(t->GetState(LOOK));

                XMVECTOR C = XMVector3Cross(R, U);                // LH면 C(=R×U)가 L과 같아야 함
                float dotRUx = XMVectorGetX(XMVector3Dot(C, L));  // ≈ +1이면 정상(LH), ≈ -1이면 축이 반대로 들어옴(미러/뒤집힘)

                _float3 r, u, l;
                XMStoreFloat3(&r, R); XMStoreFloat3(&u, U); XMStoreFloat3(&l, L);

            };

        if (GetOpenFileNameA(&ofn))
        {
            std::vector<MAP_OBJECTDATA> mapObjects =
                m_pEngineUtility->LoadMapData(szFile);

            Layer* pLayer = m_pEngineUtility->FindLayer(SCENE::MAP, TEXT("FieldObject"));
            for (auto& obj : mapObjects)
            {
                _uint iIndex = m_pEngineUtility->GetLayerSize(SCENE::MAP, TEXT("FieldObject"));
                m_pEngineUtility->AddObject(SCENE::MAP, TEXT("FieldObject"), SCENE::MAP, TEXT("FieldObject"));
                Object* pObject = m_pEngineUtility->FindObject(SCENE::MAP, TEXT("FieldObject"), iIndex);
                if (!pObject)
                    continue;

                // 모델 로드
                Model* pModel = dynamic_cast<Model*>(pObject->FindComponent(TEXT("Model")));
                if (pModel)
                {
                    ModelData* pModelData = m_pEngineUtility->LoadNoAssimpModel(obj.modelPath.c_str());
                    if (pModelData)
                    {
                        pModel->SetModelData(pModelData);
                        pModel->SetBinPath(obj.modelPath);
                    }
                }

                // 위치 세팅
                Transform* pTransform = dynamic_cast<Transform*>(pObject->FindComponent(TEXT("Transform")));
                if (pTransform)
                {
                    _vector right = XMLoadFloat4((_float4*)&obj.worldMatrix.m[0]);
                    _vector up = XMLoadFloat4((_float4*)&obj.worldMatrix.m[1]);
                    _vector look = XMLoadFloat4((_float4*)&obj.worldMatrix.m[2]);
                    _vector pos = XMLoadFloat4((_float4*)&obj.worldMatrix.m[3]);

                    pTransform->SetState(RIGHT, right);
                    pTransform->SetState(UP, up);
                    pTransform->SetState(LOOK, look);
                    pTransform->SetState(POSITION, pos);


                    DumpBasis(pTransform);
                }
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Map Data"))
    {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Map Data (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

        if (GetSaveFileNameA(&ofn))
            m_pEngineUtility->SaveMapData(szFile);
    }

    ImGui::Separator();
    ImGui::Text("Scene Objects:");

    Layer* pLayer = m_pEngineUtility->FindLayer(SCENE::MAP, TEXT("FieldObject"));
    if (pLayer)
    {
        const auto& objectList = pLayer->GetAllObjects();

        if (objectList.empty())
        {
            ImGui::TextDisabled("No objects in FieldObject layer.");
        }
        else
        {
            // 리스트 UI
            ImGui::BeginChild("ObjectList", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);

            int index = 0;
            for (auto* pObj : objectList)
            {
                std::string objName{};
                objName = "Object_" + std::to_string(index);

                bool isSelected = (m_pSelectedObject == pObj);

                if (ImGui::Selectable(objName.c_str(), isSelected))
                {
                    // ✅ 선택된 오브젝트 변경
                    m_pSelectedObject = pObj;

                    // 기존 패널 제거
                    if (m_pObjectPanel)
                    {
                        m_pEngineUtility->RemovePanel(m_pObjectPanel->GetPanelName());
                        SafeRelease(m_pObjectPanel);
                        m_pObjectPanel = nullptr;
                    }

                    // 새 ObjectPanel 생성
                    std::string objPanelName = "ObjectPanel";
                    m_pObjectPanel = ObjectPanel::Create(objPanelName, true, m_pSelectedObject);
                    m_pEngineUtility->AddPanel(objPanelName, m_pObjectPanel);
                }
                ++index;
            }

            ImGui::EndChild();
        }
    }
    else
    {
        ImGui::TextDisabled("Layer 'FieldObject' not found.");
    }
}

void MapPanel::SetSelectedBinPath(const string& path)
{
    m_SelectedModelDataPath = path;
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