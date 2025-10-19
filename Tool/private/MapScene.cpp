#include "MapScene.h"

#include "EngineUtility.h"
#include "IEHelper.h"

#include "MapPanel.h"
#include "FreeCam.h"
#include "TestTerrain.h"
#include "FieldObject.h"

MapScene::MapScene()
	:Scene{}
{

}

HRESULT MapScene::Initialize()
{
	// Buffer
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_VIBuffer_Rect"), VIBufferRect::Create());
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_VIBuffer_Cube"),	VIBufferCube::Create());
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_VIBufferTerrain"), VIBufferTerrain::Create(TEXT("../bin/Resources/Textures/Terrain/Height.bmp")));

	//Shader
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Shader_VtxPosTex"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Shader_VtxTerrain"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxNorTex.hlsl"),	VTXNORTEX::Elements, VTXNORTEX::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Shader_VtxMesh"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"),	VTXMESH::Elements, VTXMESH::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxAnimMesh.hlsl"), VTXSKINMESH::Elements, VTXSKINMESH::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Shader_VtxCube"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxCube.hlsl"),	VTXCUBE::Elements, VTXCUBE::iNumElements));

	//Texture
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Texture_Terrain_Diffuse"), Texture::Create(TEXT("../bin/Resources/Textures/Terrain/Tile%d.dds"), 2));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Texture_Terrain_Mask"), Texture::Create(TEXT("../bin/Resources/Textures/Terrain/Mask.dds"), 1));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Texture_Terrain_Brush"), Texture::Create(TEXT("../bin/Resources/Textures/Terrain/Brush.png"), 1));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Texture_Default"),
		Texture::Create(TEXT("../bin/Resources/Textures/Default%d.jpg"), 2));

	//Navigation
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Navigation"), Navigation::Create(TEXT("../bin/data/Navigation.dat")));

	//Model
	ModelData* model = new ModelData();
	IEHelper::ImportFBX("../bin/Resources/Models/Fiona/Fiona.fbx", *model);
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_Component_Model_Bullet"), Model::Create(MODELTYPE::ANIM, model, PreTransformMatrix))))
		return E_FAIL;

	//Object
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_GameObject_Terrain"), TestTerrain::Create());
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_GameObject_FieldObject"), FieldObject::Create());

	m_pEngineUtility->AddObject(SCENE::MAP, TEXT("Prototype_GameObject_Terrain"), SCENE::MAP, TEXT("Terrain"));
	
	//IMGUI Panel
	string str = "MapPanel";
	MapPanel* pPanel = MapPanel::Create(str);
	m_pEngineUtility->AddPanel(pPanel->GetPanelName(), pPanel);

	//Light
	LIGHT_DESC		LightDesc{};

	LightDesc.eType = LIGHT::DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pEngineUtility->AddLight(LightDesc)))
		return E_FAIL;

	//Cam
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Prototype_GameObject_FreeCam"), FreeCam::Create())))
		return E_FAIL;

	FreeCam::FREECAM_DESC			Desc{};
	Desc.vEye = _float3(0.f, 10.f, -6.f);
	Desc.vAt = _float3(0.f, 0.f, 0.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;
	Desc.fSensor = 0.1f;
	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);

	if (FAILED(m_pEngineUtility->AddObject(SCENE::MAP, TEXT("Prototype_GameObject_FreeCam"), SCENE::MAP, TEXT("Cam"), &Desc)))
		return E_FAIL;


	return S_OK;
}
void MapScene::Update(_float fTimeDelta)
{
}
HRESULT MapScene::Render()
{
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