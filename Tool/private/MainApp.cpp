#include "MainApp.h"
#include "EngineUtility.h"

#include "ModelScene"

MainApp::MainApp()
	: m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT MainApp::Initialize()
{
	ENGINE_DESC			EngineDesc{};
	EngineDesc.hInstance = g_hInstance;
	EngineDesc.hWnd = g_hWnd;
	EngineDesc.eWinMode = WINMODE::WIN;
	EngineDesc.iWinSizeX = g_iWinSizeX;
	EngineDesc.iWinSizeY = g_iWinSizeY;
	EngineDesc.iNumLevels = SCENE::SCENE_END;

	if (FAILED(m_pEngineUtility->InitializeEngine(EngineDesc)))
		return E_FAIL;

	if (FAILED(StartScene(SCENE::MODEL)))
		return E_FAIL;

	return S_OK;
}

void MainApp::Update(_float fTimeDelta)
{
	m_pEngineUtility->UpdateEngine(fTimeDelta);
}

HRESULT MainApp::Render()
{
	_float4		vClearColor = _float4(0.f, 0.f, 1.f, 1.f);

	/* 백, 깊이버퍼를 초기화한다. */
	m_pEngineUtility->BeginDraw(&vClearColor);

	/* 객체들을 그린다. */
	m_pEngineUtility->Draw();

	m_pEngineUtility->DrawFont(TEXT("Font_Default"), TEXT("ab이거 봐라de"), _float2(0.f, 0.f));

	/* 후면버퍼를 전면으로 보여준다. */
	m_pEngineUtility->EndDraw();

	return S_OK;
}

MainApp* MainApp::Create()
{
	MainApp* pInstance = new MainApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : MainApp");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void MainApp::Free()
{
	__super::Free();

	m_pEngineUtility->ReleaseEngine();
	m_pEngineUtility->DestroyInstance();
}

HRESULT MainApp::ReadyPrototypeForStatic()
{
	return S_OK;
}

HRESULT MainApp::StartScene(SCENE eStartLevelID)
{
	return m_pEngineUtility->ChangeScene(SCENE::MODEL, ModelScene::Create(eStartLevelID));
}