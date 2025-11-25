#include "ClientApp.h"
#include "EngineUtility.h"

#include "LoadingScene.h"

#include "FreeCam.h"
#include "FixedCam.h"
#include "ChaseCam.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIButton.h"

ClientApp::ClientApp()
	: m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT ClientApp::Initialize()
{
	SetWindowText(g_hWnd, TEXT("Alien Swarm: Reactive Drop"));

	ENGINE_DESC			EngineDesc{};
	EngineDesc.hInstance = g_hInstance;
	EngineDesc.hWnd = g_hWnd;
	EngineDesc.eWinMode = WINMODE::WINMODE_WINDOW;
	EngineDesc.iWinSizeX = g_iWinSizeX;
	EngineDesc.iWinSizeY = g_iWinSizeY;
	EngineDesc.iNumLevels = SCENE::SCENE_END;

	if (FAILED(m_pEngineUtility->InitializeEngine(EngineDesc)))
		return E_FAIL;

	if (FAILED(ReadyPrototypeForStatic()))
		return E_FAIL;

	if (FAILED(StartScene(SCENE::LOGO)))
		return E_FAIL;

#ifdef _DEBUG
	UILabel* pUILabel = UILabel::Create();
	if (pUILabel)
	{
		UI_DESC desc{};
		desc.fSpeedPerSec = 0.f;
		desc.fRotationPerSec = 0.f;
		desc.type = UITYPE::UI_LABEL;
		desc.name = "FPS";
		desc.x = 20.f;
		desc.y = 20.f;
		desc.z = 0.f;
		desc.w = 100.f;
		desc.h = 20.f;
		desc.visible = true;
		desc.font = L"Font_Default";
		desc.text = L"FPS: 0";
		desc.fontSize = 20.f;
		desc.fontColor = { 1.f,1.f,1.f,1.f };

		m_pFPSLabel = dynamic_cast<UILabel*>(pUILabel->Clone(&desc));
		if (m_pFPSLabel)
		{
			m_pEngineUtility->AddUI(L"FPS", m_pFPSLabel);
		}
	}
	SafeRelease(pUILabel);
#endif
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
	//m_pEngineUtility->RenderGrid();
#endif

	/* 객체들을 그린다. */
	m_pEngineUtility->Draw();

#ifdef _DEBUG
	//m_pEngineUtility->RenderNavigation();
	//m_pEngineUtility->RenderTriggerBoxes();
	//m_pEngineUtility->RenderDebug();
	m_pEngineUtility->FindUI(L"FPS")->Render();
#endif

	/* 후면버퍼를 전면으로 보여준다. */
	m_pEngineUtility->EndDraw();

	m_pEngineUtility->ClearDeadObjects();

	return S_OK;
}

#ifdef _DEBUG
void ClientApp::SetFPS(_uint fps)
{
	if (m_pFPSLabel == nullptr)
		return;

	wstring text = L"FPS: "+ to_wstring(fps);
	m_pFPSLabel->SetText(text);
}
#endif

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

#ifdef _DEBUG
	SafeRelease(m_pFPSLabel);
#endif

	m_pEngineUtility->ReleaseEngine();
	m_pEngineUtility->DestroyInstance();

}

HRESULT ClientApp::ReadyPrototypeForStatic()
{
	if (FAILED(m_pEngineUtility->AddFont(L"Font_Default", L"../bin/Resources/Fonts/155ex.spritefont")))
		return E_FAIL;

	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxPosTex"), Shader::Create(TEXT("../bin/Shader/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxAnimMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxAnimMesh.hlsl"), VTXSKINMESH::Elements, VTXSKINMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_InstancingPos"), Shader::Create(TEXT("../bin/Shader/Shader_InstancingPos.hlsl"), VTXPOS_INSTANCEPARTICLE::Elements, VTXPOS_INSTANCEPARTICLE::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxProjectile"), Shader::Create(TEXT("../bin/Shader/Shader_VtxProjectile.hlsl"), VTXPOS_INSTANCEPARTICLE::Elements, VTXPOS_INSTANCEPARTICLE::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxBloodHit"), Shader::Create(TEXT("../bin/Shader/Shader_VtxBloodHit.hlsl"), VTXPOSTEX_INSTANCEWORLD::Elements, VTXPOSTEX_INSTANCEWORLD::iNumElements))))
		return E_FAIL;

	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("VIBufferRect"), VIBufferRect::Create())))
		return E_FAIL;
	
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("StateMachine"), StateMachine::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("AIController"), AIController::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Info"), Info::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Physics"), Physics::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("CollisionAABB"), Collision::Create(COLLISIONTYPE_AABB))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("CollisionOBB"), Collision::Create(COLLISIONTYPE_OBB))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("CollisionSphere"), Collision::Create(COLLISIONTYPE_SPHERE))))
		return E_FAIL;

	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("FreeCam"), FreeCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("FixedCam"), FixedCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("ChaseCam"), ChaseCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("UIImage"), UIImage::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("UILabel"), UILabel::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("UIButton"), UIButton::Create())))
		return E_FAIL;

	return S_OK;
}

HRESULT ClientApp::StartScene(SCENE eStartLevelID)
{
	return m_pEngineUtility->ChangeScene(SCENE::LOADING, LoadingScene::Create(eStartLevelID));
}
