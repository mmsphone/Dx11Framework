#include "SceneManager.h"
#include "Scene.h"
#include "EngineUtility.h"


SceneManager::SceneManager()
	:m_pEngineUtility { EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT SceneManager::ChangeScene(_uint iSceneId, Scene* pScene)
{
#ifdef _DEBUG
	OutputDebugStringA("Clearing Scene...\n");
#endif
	CHECKPTR(m_pCurrentScene)
		m_pEngineUtility->ClearScene(m_iCurrentSceneId);
	SafeRelease(m_pCurrentScene);

	m_pCurrentScene = pScene;
	m_iCurrentSceneId = iSceneId;

	return S_OK;
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

_uint SceneManager::GetCurrentSceneId() const
{
	return m_iCurrentSceneId;
}

SceneManager* SceneManager::Create()
{
	return new SceneManager();
}

void SceneManager::Free()
{
	__super::Free();

	m_pEngineUtility->DestroyInstance();

	SafeRelease(m_pCurrentScene);
}
