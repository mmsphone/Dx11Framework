#include "Loader.h"

#include "EngineUtility.h"

#include "FreeCam.h"
#include "Background.h"
#include "Monster.h"
#include "Terrain.h"
#include "Sky.h"

#include "Player.h"
#include "PlayerBody.h"
#include "PlayerWeapon.h"

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
	lstrcpy(m_szLoading, TEXT("텍스쳐를 로딩중입니다."));
	/* for.Prototype_Component_Texture_BackGround */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Prototype_Component_Texture_BackGround"), Texture::Create(TEXT("../bin/Resources/Textures/Default%d.jpg"), 2))))
		return E_FAIL;
	
	
	lstrcpy(m_szLoading, TEXT("모델를(을) 로딩중입니다."));
	/* for.Prototype_Component_VIBuffer_Rect */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Prototype_Component_VIBuffer_Rect"), VIBufferRect::Create())))
		return E_FAIL;
	
	lstrcpy(m_szLoading, TEXT("셰이더를(을) 로딩중입니다."));
	/* for.Prototype_Component_Shader_VtxPosTex */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Prototype_Component_Shader_VtxPosTex"), Shader::Create(TEXT("../bin/Shader/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	
	
	lstrcpy(m_szLoading, TEXT("객체원형를(을) 로딩중입니다."));
	/* for.Prototype_GameObject_BackGround */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Prototype_GameObject_BackGround"), Background::Create())))
		return E_FAIL;
	
	lstrcpy(m_szLoading, TEXT("로딩이 완료되었습니다."));
	m_isFinished = true;

	return S_OK;
}

HRESULT Loader::LoadingForGamePlay()
{
	lstrcpy(m_szLoading, TEXT("텍스쳐를 로딩중입니다."));
	/* for.Prototype_Component_Texture_Terrain */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Diffuse"), Texture::Create(TEXT("../bin/Resources/Textures/Terrain/Tile%d.dds"), 2))))
		return E_FAIL;
	
	/* for.Prototype_Component_Texture_Terrain_Mask */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Mask"), Texture::Create(TEXT("../bin/Resources/Textures/Terrain/Mask.dds"), 1))))
		return E_FAIL;
	
	/* for.Prototype_Component_Texture_Terrain_Brush */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Brush"), Texture::Create(TEXT("../bin/Resources/Textures/Terrain/Brush.png"), 1))))
		return E_FAIL;
	
	/* for.Prototype_Component_Texture_Sky*/
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Texture_Sky"), Texture::Create(TEXT("../bin/Resources/Textures/Skybox/Sky_%d.dds"), 4))))
		return E_FAIL;
	
	lstrcpy(m_szLoading, TEXT("모델를(을) 로딩중입니다."));
	/* for.Prototype_Component_VIBuffer_Terrain */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_VIBufferTerrain"), VIBufferTerrain::Create(TEXT("../bin/Resources/Textures/Terrain/Height.bmp")))))
		return E_FAIL;
	
	/* for.Prototype_Component_VIBuffer_Cube*/
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_VIBufferCube"), VIBufferCube::Create())))
		return E_FAIL;
	
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	
	/* for.Prototype_Component_Model_Fiona */
	PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.0f));
	
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Model_Fiona"), Model::Create(MODELTYPE::ANIM, "../bin/Resources/Models/Fiona/Fiona.fbx", PreTransformMatrix))))
		return E_FAIL;
	
	/* for.Prototype_Component_Model_ForkLift */
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.0f));
	
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Model_ForkLift"), Model::Create(MODELTYPE::NONANIM, "../bin/Resources/Models/ForkLift/ForkLift.fbx", PreTransformMatrix))))
		return E_FAIL;
	
	lstrcpy(m_szLoading, TEXT("셰이더를(을) 로딩중입니다."));
	/* for.Prototype_Component_Shader_VtxNorTex */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Shader_VtxNorTex"), Shader::Create(TEXT("../bin/Shader/Shader_VtxNorTex.hlsl"), VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
		return E_FAIL;
	
	/* for.Prototype_Component_Shader_VtxMesh */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Shader_VtxMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	
	/* for.Prototype_Component_Shader_VtxAnimMesh */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Shader_VtxAnimMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxAnimMesh.hlsl"), VTXSKINMESH::Elements, VTXSKINMESH::iNumElements))))
		return E_FAIL;
	
	/* for.Prototype_Component_Shader_VtxCube */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_Component_Shader_VtxCube"), Shader::Create(TEXT("../bin/Shader/Shader_VtxCube.hlsl"), VTXCUBE::Elements, VTXCUBE::iNumElements))))
		return E_FAIL;
	
	
	lstrcpy(m_szLoading, TEXT("객체원형를(을) 로딩중입니다."));
	/* for.Prototype_GameObject_Terrain */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_GameObject_Terrain"), Terrain::Create())))
		return E_FAIL;
	
	/* for.Prototype_GameObject_Camera_Free */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_GameObject_FreeCam"), FreeCam::Create())))
		return E_FAIL;
	
	/* for.Prototype_GameObject_Monster */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_GameObject_Monster"), Monster::Create())))
		return E_FAIL;
	
	/* for.Prototype_GameObject_Sky */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_GameObject_Sky"), Sky::Create())))
		return E_FAIL;
	
	/* for.Prototype_GameObject_Player */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_GameObject_Player"), Player::Create())))
		return E_FAIL;
	
	
	/* for.Prototype_GameObject_Player_Body */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_GameObject_PlayerBody"), PlayerBody::Create())))
		return E_FAIL;
	
	/* for.Prototype_GameObject_Player_Weapon */
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Prototype_GameObject_PlayerWeapon"), PlayerWeapon::Create())))
		return E_FAIL;
	
	lstrcpy(m_szLoading, TEXT("로딩이 완료되었습니다."));
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
