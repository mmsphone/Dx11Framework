#include "ModelScene.h"

#include "EngineUtility.h"
#include "IEHelper.h"

#include "TestObject.h"
#include "FreeCam.h"

#include "ModelPanel.h"
#include "CamPanel.h"

ModelScene::ModelScene()
	:Scene{}
{

}

HRESULT ModelScene::Initialize()
{
	// Buffer
	m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("VIBuffer_Rect"),	VIBufferRect::Create());
	m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("VIBuffer_Cube"),	VIBufferCube::Create());
	
	//Shader
	m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("Shader_VtxPosTex"), 
		Shader::Create(TEXT("../bin/Shader/Shader_VtxPosTex.hlsl"),	VTXPOSTEX::Elements, VTXPOSTEX::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("Shader_VtxNorTex"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxNorTex.hlsl"),	VTXNORTEX::Elements, VTXNORTEX::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("Shader_VtxMesh"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"),	VTXMESH::Elements, VTXMESH::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("Shader_VtxAnimMesh"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxAnimMesh.hlsl"),	VTXSKINMESH::Elements, VTXSKINMESH::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("Shader_VtxCube"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxCube.hlsl"),	VTXCUBE::Elements, VTXCUBE::iNumElements));
	
	//Model
	ModelData* model = new ModelData();
	IEHelper::ImportFBX("../bin/Resources/Models/Fiona/Fiona.fbx", *model);
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("Model_Fiona"), Model::Create(MODELTYPE::ANIM, model, PreTransformMatrix))))
		return E_FAIL;
	//Object
	m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("TestObject"), TestObject::Create());
	m_pEngineUtility->AddObject(SCENE::MODEL, TEXT("TestObject"), SCENE::MODEL, TEXT("TestObject"));

	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::MODEL, TEXT("FreeCam"), FreeCam::Create())))
		return E_FAIL;

	FreeCam::FREECAM_DESC			Desc{};
	Desc.vEye = _float3(0.f, 0.f, 0.f);
	Desc.vAt = _float3(0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 50000.f;
	Desc.fSensor = 0.1f;
	Desc.fSpeedPerSec = 40.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);

	if (FAILED(m_pEngineUtility->AddObject(SCENE::MODEL, TEXT("FreeCam"), SCENE::MODEL, TEXT("Cam"), &Desc)))
		return E_FAIL;

	//Light
	LIGHT_DESC		LightDesc{};

	LightDesc.eType = LIGHT::DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pEngineUtility->AddLight(LightDesc)))
		return E_FAIL;

	//IMGUI Panel
	string strModelPanel = "ModelPanel";
	ModelPanel* pModelPanel = ModelPanel::Create(strModelPanel);
	m_pEngineUtility->AddPanel(pModelPanel->GetPanelName(), pModelPanel);

	string strCamPanel = "ModelCamPanel";
	CamPanel* pCamPanel = CamPanel::Create(strCamPanel, SCENE::MODEL);
	m_pEngineUtility->AddPanel(pCamPanel->GetPanelName(), pCamPanel);

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