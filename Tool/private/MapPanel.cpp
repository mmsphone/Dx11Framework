#include "MapPanel.h"

#include "Tool_Defines.h"

#include "EngineUtility.h"
#include "ModelScene.h"

MapPanel::MapPanel(const string& PanelName, bool open)
    :Panel{ PanelName, open }
{

}

void MapPanel::OnRender()
{
    if (ImGui::Button("Move To ModelScene"))
    {
        m_pEngineUtility->ChangeScene(SCENE::MODEL, ModelScene::Create(SCENE::MODEL));
        m_pEngineUtility->SetPanelOpen(GetPanelName(), false);
    }

    ImGui::Text("Click on terrain to place marker");
}


MapPanel* MapPanel::Create(const string& PanelName, bool open)
{
    return new MapPanel(PanelName, open);
}

void MapPanel::Free()
{
    __super::Free();
}