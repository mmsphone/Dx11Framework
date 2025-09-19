#include "SceneManager.h"
#include "Scene.h"
#include "EngineUtility.h"


SceneManager::SceneManager()
	:m_pEngineUtility { EngineUtility::GetInstance() }
{
	Safe_AddRef(m_pEngineUtility);
}

HRESULT SceneManager::ChangeScene(_uint iSceneId, Scene* pScene)
{
	CHECKPTR(m_pCurrentScene)
		m_pEngineUtility->ClearScene(m_iSceneId);
	Safe_Release(m_pCurrentScene);

	m_pCurrentScene = pScene;
	m_iSceneId = iSceneId;

	return E_NOTIMPL;
}

void SceneManager::Update(_float fTimeDelta)
{
	CHECKNULLPTR(m_pCurrentScene)
		return;

	m_pCurrentScene->Update(fTimeDelta);
}

HRESULT SceneManager::Render()
{
	CHECKNULLPTR(m_pCurrentScene)
		return E_FAIL;

	return m_pCurrentScene->Render();
}

SceneManager* SceneManager::Create()
{
	return new SceneManager();
}

void SceneManager::Free()
{
	__super::Free();

	m_pEngineUtility->DestroyInstance();

	Safe_Release(m_pCurrentScene);
}
