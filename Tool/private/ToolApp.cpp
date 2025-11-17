#include "ToolApp.h"
#include "EngineUtility.h"

#include "ModelScene.h"

ToolApp::ToolApp()
	: m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT ToolApp::Initialize()
{
	std::filesystem::current_path("../bin");
	
	ENGINE_DESC			EngineDesc{};
	EngineDesc.hInstance = g_hInstance;
	EngineDesc.hWnd = g_hWnd;
	EngineDesc.eWinMode = WINMODE::WINMODE_WINDOW;
	EngineDesc.iWinSizeX = g_iWinSizeX;
	EngineDesc.iWinSizeY = g_iWinSizeY;
	EngineDesc.iNumLevels = SCENE::SCENE_END;

	if (FAILED(m_pEngineUtility->InitializeEngine(EngineDesc)))
		return E_FAIL;

	ImGui::SetCurrentContext(m_pEngineUtility->GetIMGUIContext());

	if (FAILED(m_pEngineUtility->ChangeScene(SCENE::MODEL, ModelScene::Create(SCENE::MODEL))))
		return E_FAIL;

	return S_OK;
}

void ToolApp::Update(_float fTimeDelta)
{
	m_pEngineUtility->BeginIMGUI();
		
	m_pEngineUtility->UpdateEngine(fTimeDelta);

	m_pEngineUtility->DrawPanels();
}

HRESULT ToolApp::Render()
{
	_float4		vClearColor = _float4(0.f, 0.f, 0.f, 1.f);

	/* 백, 깊이버퍼를 초기화한다. */
	m_pEngineUtility->BeginDraw(&vClearColor);

#ifdef _DEBUG
	//m_pEngineUtility->RenderGrid();
#endif
	/* 객체들을 그린다. */
	m_pEngineUtility->Draw();

#ifdef _DEBUG
	//m_pEngineUtility->RenderNavigation();
	//m_pEngineUtility->RenderTriggerBoxes();
	//m_pEngineUtility->RenderDebug();
#endif

	m_pEngineUtility->RenderIMGUI();

	/* 후면버퍼를 전면으로 보여준다. */
	m_pEngineUtility->EndDraw();

	m_pEngineUtility->ClearDeadObjects();

	return S_OK;
}

ToolApp* ToolApp::Create()
{
	ToolApp* pInstance = new ToolApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : MainApp");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void ToolApp::Free()
{
	__super::Free();

	m_pEngineUtility->ReleaseEngine();
	m_pEngineUtility->DestroyInstance();
}

HRESULT ToolApp::ReadyPrototypeForStatic()
{
	return S_OK;
}