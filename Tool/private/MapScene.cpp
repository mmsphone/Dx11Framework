#include "MapScene.h"

#include "EngineUtility.h"
#include "IEHelper.h"

#include "MapPanel.h"
#include "FreeCam.h"
#include "TestTerrain.h"
#include "FieldObject.h"
#include "CamPanel.h"
#include "AssetPanel.h"
#include "LightPanel.h"
#include "TriggerBoxPanel.h"
#include "UIPanel.h"

MapScene::MapScene()
	:Scene{}
{

}

HRESULT MapScene::Initialize()
{
	// Buffer
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("VIBuffer_Rect"), VIBufferRect::Create());
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("VIBuffer_Cube"),	VIBufferCube::Create());
	//m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("VIBufferTerrain"), 
	//	VIBufferTerrain::Create(TEXT("../bin/Resources/Textures/Terrain/Height.bmp")));

	//Shader
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Shader_VtxPosTex"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements));
	//m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Shader_VtxTerrain"),
	//	Shader::Create(TEXT("../bin/Shader/Shader_VtxNorTex.hlsl"),	VTXNORTEX::Elements, VTXNORTEX::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Shader_VtxMesh"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"),	VTXMESH::Elements, VTXMESH::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Shader_VtxAnimMesh"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxAnimMesh.hlsl"), VTXSKINMESH::Elements, VTXSKINMESH::iNumElements));
	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Shader_VtxCube"),
		Shader::Create(TEXT("../bin/Shader/Shader_VtxCube.hlsl"),	VTXCUBE::Elements, VTXCUBE::iNumElements));

	//Texture
	
	//Collision
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("CollisionAABB"), Collision::Create(COLLISIONTYPE_AABB))))
		return E_FAIL;

	//Model
	ModelData* model = new ModelData();
	IEHelper::ImportFBX("../bin/Resources/Models/Fiona/Fiona.fbx", *model);
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Model_Fiona"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model, PreTransformMatrix))))
		return E_FAIL;
	

	//Object
	//m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("Terrain"), TestTerrain::Create());
	//m_pEngineUtility->AddObject(SCENE::MAP, TEXT("Terrain"), SCENE::MAP, TEXT("Terrain"));

	m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("FieldObject"), FieldObject::Create());

	//Cam
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::MAP, TEXT("FreeCam"), FreeCam::Create())))
		return E_FAIL;
	
	FreeCam::FREECAM_DESC			Desc{};
	Desc.vEye = _float3(0.f, 10.f, -6.f);
	Desc.vAt = _float3(0.f, 0.f, 0.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;
	Desc.fSensor = 0.1f;
	Desc.fSpeedPerSec = 40.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);
	
	if (FAILED(m_pEngineUtility->AddObject(SCENE::MAP, TEXT("FreeCam"), SCENE::MAP, TEXT("Cam"), &Desc)))
		return E_FAIL;
	Camera* pCam = static_cast<Camera*>(m_pEngineUtility->FindObject(SCENE::MAP, TEXT("Cam"), 0));
	m_pEngineUtility->SetMainCamera(pCam);

	//IMGUI Panel
	string str = "MapPanel";
	MapPanel* pPanel = MapPanel::Create(str);
	m_pEngineUtility->AddPanel(pPanel->GetPanelName(), pPanel);

	string strCamPanel = "MapCamPanel";
	CamPanel* pCamPanel = CamPanel::Create(strCamPanel, SCENE::MAP);
	m_pEngineUtility->AddPanel(pCamPanel->GetPanelName(), pCamPanel);

	string strAssetPanel = "AssetPanel";
	AssetPanel* pAssetPanel = AssetPanel::Create(strAssetPanel);
	m_pEngineUtility->AddPanel(pAssetPanel->GetPanelName(), pAssetPanel);

	string strLightPanel = "LightPanel";
	LightPanel* pLightPanel = LightPanel::Create(strLightPanel);
	m_pEngineUtility->AddPanel(pLightPanel->GetPanelName(), pLightPanel);

	string strTriggerBoxPanel = "TiggerBoxPanel";
	TriggerBoxPanel* pTriggerBoxPanel = TriggerBoxPanel::Create(strTriggerBoxPanel);
	m_pEngineUtility->AddPanel(pTriggerBoxPanel->GetPanelName(), pTriggerBoxPanel);

	string strUIPanel = "UIPanel";
	UIPanel* pUIPanel = UIPanel::Create(strUIPanel);
	m_pEngineUtility->AddPanel(pUIPanel->GetPanelName(), pUIPanel);

	return S_OK;
}
void MapScene::Update(_float fTimeDelta)
{
}
HRESULT MapScene::Render()
{
	auto [target, op] = m_pEngineUtility->GetGizmoState();
	if (!target || target->IsDead())
		return S_OK;

	Transform* pTransform = dynamic_cast<Transform*>(target->FindComponent(TEXT("Transform")));
	if (!pTransform)
		return S_OK;

	// --- [1] 기즈모 컨텍스트 세팅 (현재 ImGui 프레임 내부로) ---
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::BeginFrame(); // ✅ “Debug” 패널 생성을 막는 핵심 한 줄

	ImGuiIO& io = ImGui::GetIO();
	io.WantCaptureMouse = false; // 마우스 캡처 방지

	auto winSize = m_pEngineUtility->GetWindowSize();
	ImGuizmo::SetRect(0, 0, (float)winSize.x, (float)winSize.y);
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList()); // ✅ 전역 Foreground DrawList에 그림

	// --- [2] 행렬 세팅 ---
	_float4x4 view = *m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW);
	_float4x4 proj = *m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION);
	_float4x4 world = *pTransform->GetWorldMatrixPtr();

	// DirectX 행렬은 row-major 이므로 transpose 필요 X (보통 그대로 써야 함)
	// 만약 화면 비틀리면 XMMatrixTranspose 적용해보기

	// --- [3] 기즈모 조작 ---
	ImGuizmo::Manipulate(
		reinterpret_cast<float*>(&view),
		reinterpret_cast<float*>(&proj),
		op,
		ImGuizmo::WORLD,
		reinterpret_cast<float*>(&world)
	);

	// --- [4] 결과 반영 ---
	if (ImGuizmo::IsUsing())
	{
		XMMATRIX xmWorld = XMLoadFloat4x4(&world);
		XMVECTOR s, r, t;
		XMMatrixDecompose(&s, &r, &t, xmWorld);

		XMVECTOR right = xmWorld.r[0];
		XMVECTOR up = xmWorld.r[1];
		XMVECTOR look = xmWorld.r[2];

		pTransform->SetState(MATRIXROW_RIGHT, right);
		pTransform->SetState(MATRIXROW_UP, up);
		pTransform->SetState(MATRIXROW_LOOK, look);
		pTransform->SetState(MATRIXROW_POSITION, t);
	}

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