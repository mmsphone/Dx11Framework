#include "MapScene.h"

#include "EngineUtility.h"

#include "MapPanel.h"

MapScene::MapScene()
	:Scene{}
{

}

HRESULT MapScene::Initialize()
{
	string str = "MapPanel";
	MapPanel* pPanel = MapPanel::Create(str);
	m_pEngineUtility->AddPanel(pPanel->GetPanelName(), pPanel);

	// TODO: Grid, Terrain, Marker 초기화

	return S_OK;
}
void MapScene::Update(_float fTimeDelta)
{
	// TODO: 마우스 피킹 및 마커 이동
}
HRESULT MapScene::Render()
{

	// TODO: Grid / Terrain / Marker 렌더링

	return S_OK;
}

MapScene* MapScene::Create(SCENE eStartLevelID)
{
	MapScene* pInstance = new MapScene();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : MapScene");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void MapScene::Free()
{
	__super::Free();
}