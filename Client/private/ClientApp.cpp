#include "ClientApp.h"
#include "EngineUtility.h"

#include "LoadingScene.h"

ClientApp::ClientApp()
	: m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT ClientApp::Initialize()
{
	ENGINE_DESC			EngineDesc{};
	EngineDesc.hInstance = g_hInstance;
	EngineDesc.hWnd = g_hWnd;
	EngineDesc.eWinMode = WINMODE::WINMODE_WINDOW;
	EngineDesc.iWinSizeX = g_iWinSizeX;
	EngineDesc.iWinSizeY = g_iWinSizeY;
	EngineDesc.iNumLevels = SCENE::SCENE_END;

	if (FAILED(m_pEngineUtility->InitializeEngine(EngineDesc)))
		return E_FAIL;

	if (FAILED(StartScene(SCENE::LOGO)))
		return E_FAIL;

	return S_OK;
}

void ClientApp::Update(_float fTimeDelta)
{
	m_pEngineUtility->UpdateEngine(fTimeDelta);
}

HRESULT ClientApp::Render()
{
	_float4		vClearColor = _float4(0.f, 0.f, 0.f, 1.f);

	/* 백, 깊이버퍼를 초기화한다. */
	m_pEngineUtility->BeginDraw(&vClearColor);

#ifdef _DEBUG
	m_pEngineUtility->RenderGrid();
#endif

	/* 객체들을 그린다. */
	m_pEngineUtility->Draw();

#ifdef _DEBUG
	//m_pEngineUtility->RenderNavigation();
	m_pEngineUtility->RenderTriggerBoxes();
	//m_pEngineUtility->RenderDebug();
#endif

	/* 후면버퍼를 전면으로 보여준다. */
	m_pEngineUtility->EndDraw();

	m_pEngineUtility->ClearDeadObjects();

	return S_OK;
}

ClientApp* ClientApp::Create()
{
	ClientApp* pInstance = new ClientApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : ClientApp");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void ClientApp::Free()
{
	__super::Free();

	m_pEngineUtility->ReleaseEngine();
	m_pEngineUtility->DestroyInstance();
}

HRESULT ClientApp::ReadyPrototypeForStatic()
{
	return S_OK;
}

HRESULT ClientApp::StartScene(SCENE eStartLevelID)
{
	return m_pEngineUtility->ChangeScene(SCENE::LOADING, LoadingScene::Create(eStartLevelID));
}
