#include "ModelScene.h"

#include "EngineUtility.h"

#include "ModelPanel.h"

ModelScene::ModelScene()
	:Scene{}
{

}

HRESULT ModelScene::Initialize()
{
	string str = "ModelPanel";
	ModelPanel* pPanel = ModelPanel::Create(str);
	m_pEngineUtility->AddPanel(pPanel->GetPanelName(), pPanel);
	return S_OK;
}
void ModelScene::Update(_float fTimeDelta)
{
}
HRESULT ModelScene::Render()
{
	return S_OK;
}

ModelScene* ModelScene::Create(SCENE eStartLevelID)
{
	ModelScene* pInstance = new ModelScene();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : ModelScene");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void ModelScene::Free()
{
	__super::Free();
}