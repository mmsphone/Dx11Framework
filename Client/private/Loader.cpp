#include "Loader.h"

#include "EngineUtility.h"

#include "FreeCam.h"
#include "Background.h"
#include "Ground.h"
#include "Sky.h"
#include "CarWheel.h"
#include "CarTrunk.h"
#include "CarTrunkHandle.h"
#include "CarTopStuff.h"
#include "CarBody.h"
#include "TreeHead.h"
#include "Plant.h"
#include "HouseDoorHandle.h"
#include "HouseDoor.h"
#include "WoodenFence.h"
#include "CageCamera.h"
#include "UIStart.h"
#include "FixedCam.h"

Loader::Loader()
	: m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

_uint APIENTRY ThreadMain(void* pArg)
{
	Loader* pLoader = static_cast<Loader*>(pArg);

	if (FAILED(pLoader->Loading()))
		return 1;

	return 0;
}

HRESULT Loader::Initialize(SCENE eNextSceneId)
{
	m_eNextLevelID = eNextSceneId;

	InitializeCriticalSection(&m_CriticalSection);

	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;

	return S_OK;
}

HRESULT Loader::Loading()
{
	/* 컴객체를 초기화한다. */
	CoInitializeEx(nullptr, 0);

	EnterCriticalSection(&m_CriticalSection);

	HRESULT		hr = {};

	/* 이제 여기에서 레벨에 맞는 로딩을 수행하면 된다. */

	switch (m_eNextLevelID)
	{
	case SCENE::LOGO:
		hr = LoadingForLogo();
		break;
	case SCENE::GAMEPLAY:
		hr = LoadingForGamePlay();
		break;
	}

	LeaveCriticalSection(&m_CriticalSection);
	CoUninitialize();

	return hr;
}

_bool Loader::isFinished() const
{
	return m_isFinished;
}

void Loader::PrintText()
{
	SetWindowText(g_hWnd, m_szLoading);
}

HRESULT Loader::LoadingForLogo()
{
	lstrcpy(m_szLoading, TEXT("텍스처 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Texture_UIStart"), Texture::Create(TEXT("../bin/Resources/Textures/UIStart.png"), 1))))
		return E_FAIL;
	lstrcpy(m_szLoading, TEXT("셰이더 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Shader_VtxMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Shader_VtxPosTex"), Shader::Create(TEXT("../bin/Shader/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	lstrcpy(m_szLoading, TEXT("모델 로딩중..."));
	ModelData* model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/ItemInCage/ItemInCage_Mesh0.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Model_CageCamera"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("VIBuffer_Rect"), VIBufferRect::Create())))
		return E_FAIL;
	
	lstrcpy(m_szLoading, TEXT("객체원형 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("FixedCam"), FixedCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("CageCamera"), CageCamera::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("UIStart"), UIStart::Create())))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("로딩 완료"));
	m_isFinished = true;

	return S_OK;
}

HRESULT Loader::LoadingForGamePlay()
{
	lstrcpy(m_szLoading, TEXT("텍스처 로딩중..."));

	lstrcpy(m_szLoading, TEXT("셰이더 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Shader_SphereSky"), Shader::Create(TEXT("../bin/Shader/Shader_SphereSky.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Shader_VtxMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("모델 로딩중..."));
	ModelData* model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh33.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Sky"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh30.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Ground"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh0.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_CarWheel"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh5.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_CarTrunk"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh6.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_CarTrunkHandle"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh7.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_CarTopStuff"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh4.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_CarBody"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh31.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Plant"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh32.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_TreeHead"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh13.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_HouseDoorHandle"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh12.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_HouseDoor"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Ground4/Ground4_Mesh26.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_WoodenFence"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;


	lstrcpy(m_szLoading, TEXT("객체원형 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("FreeCam"), FreeCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Sky"), Sky::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Ground"), Ground::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("CarWheel"), CarWheel::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("CarTrunk"), TreeHead::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("CarTrunkHandle"), TreeHead::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("CarTopStuff"), TreeHead::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("CarBody"), CarBody::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Plant"), Plant::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("TreeHead"), TreeHead::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("HouseDoorHandle"), HouseDoorHandle::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("HouseDoor"), HouseDoor::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("WoodenFence"), WoodenFence::Create())))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("로딩 완료"));
	m_isFinished = true;

	return S_OK;
}

Loader* Loader::Create(SCENE eNextSceneId)
{
	Loader* pInstance = new Loader();

	if (FAILED(pInstance->Initialize(eNextSceneId)))
	{
		MSG_BOX("Failed to Created : Loader");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void Loader::Free()
{
	__super::Free();

	WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);
	DeleteCriticalSection(&m_CriticalSection);

	m_pEngineUtility->DestroyInstance();
}
