#include "EngineUtility.h"

#include "Graphic.h"
#include "Input.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "ObjectManager.h"
#include "PrototypeManager.h"
#include "RenderManager.h"
#include "Pipeline.h"
#include "LightManager.h"
#include "FontManager.h"
#include "IMGUIManager.h"
#include "PickingManager.h"
#include "GridManager.h"
#include "NavigationManager.h"
#include "SaveLoadManager.h"
#include "RenderTargetManager.h"
#include "ShadowLightManager.h"
#include "SpawnerManager.h"
#include "TriggerBoxManager.h"
#include "Frustum.h"

IMPLEMENT_SINGLETON(EngineUtility)

EngineUtility::EngineUtility()
{
}

HRESULT EngineUtility::InitializeEngine(const ENGINE_DESC& EngineDesc)
{
	m_pGraphic = Graphic::Create(EngineDesc.hWnd, EngineDesc.eWinMode, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY);
	CHECKNULLPTR(m_pGraphic) return E_FAIL;

	m_pInput = Input::Create(EngineDesc.hInstance, EngineDesc.hWnd);
	CHECKNULLPTR(m_pInput) return E_FAIL;

	m_pRenderTargetManager = RenderTargetManager::Create();
	CHECKNULLPTR(m_pRenderTargetManager) return E_FAIL;

	m_pRenderManager = RenderManager::Create();
	CHECKNULLPTR(m_pRenderManager) return E_FAIL;

	m_pTimeManager = TimeManager::Create();
	CHECKNULLPTR(m_pTimeManager) return E_FAIL;

	m_pPrototypeManager = PrototypeManager::Create(EngineDesc.iNumLevels);
	CHECKNULLPTR(m_pPrototypeManager) return E_FAIL;

	m_pObjectManager = ObjectManager::Create(EngineDesc.iNumLevels);
	CHECKNULLPTR(m_pObjectManager) return E_FAIL;

	m_pSceneManager = SceneManager::Create();
	CHECKNULLPTR(m_pSceneManager) return E_FAIL;

	m_pPipeLine = Pipeline::Create();
	CHECKNULLPTR(m_pPipeLine) return E_FAIL;

	m_pLightManager = LightManager::Create();
	CHECKNULLPTR(m_pLightManager) return E_FAIL;

	m_pFontManager = FontManager::Create();
	CHECKNULLPTR(m_pFontManager) return E_FAIL;

	m_pIMGUIManager = IMGUIManager::Create(EngineDesc.hWnd);
	CHECKNULLPTR(m_pIMGUIManager) return E_FAIL;

	m_pPickingManager = PickingManager::Create();
	CHECKNULLPTR(m_pPickingManager) return E_FAIL;

	m_pGridManager = GridManager::Create();
	CHECKNULLPTR(m_pGridManager) return E_FAIL;

	m_pNavigationManager = NavigationManager::Create();
	CHECKNULLPTR(m_pNavigationManager) return E_FAIL;

	m_pSaveLoadManager = SaveLoadManager::Create();
	CHECKNULLPTR(m_pSaveLoadManager) return E_FAIL;

	m_pShadowLightManager = ShadowLightManager::Create();
	CHECKNULLPTR(m_pShadowLightManager) return E_FAIL;

	m_pSpawnerManager = SpawnerManager::Create();
	CHECKNULLPTR(m_pSpawnerManager) return E_FAIL;

	m_pTriggerBoxManager = TriggerBoxManager::Create();
	CHECKNULLPTR(m_pTriggerBoxManager) return E_FAIL;

	m_pFrustum = Frustum::Create();
	CHECKNULLPTR(m_pFrustum) return E_FAIL;

	return S_OK;
}

void EngineUtility::UpdateEngine(_float fTimeDelta)
{
	m_pInput->Update();

	m_pObjectManager->PriorityUpdate(fTimeDelta);
	m_pPipeLine->Update();
	m_pObjectManager->Update(fTimeDelta);
	m_pTriggerBoxManager->UpdateTriggers();
	m_pObjectManager->LateUpdate(fTimeDelta);

	m_pSceneManager->Update(fTimeDelta);
}

HRESULT EngineUtility::BeginDraw(const _float4* pColor)
{
	m_pGraphic->ClearBackBufferView(pColor);
	m_pGraphic->ClearDepthStencilView();

	return S_OK;
}

HRESULT EngineUtility::Draw()
{
	m_pRenderManager->Draw();
	m_pSceneManager->Render();

	return S_OK;
}

HRESULT EngineUtility::EndDraw()
{
	m_pGraphic->Present();

	return S_OK;
}

void EngineUtility::ReleaseEngine()
{
	SafeRelease(m_pFrustum);
	SafeRelease(m_pTriggerBoxManager);
	SafeRelease(m_pSpawnerManager);
	SafeRelease(m_pShadowLightManager);
	SafeRelease(m_pSaveLoadManager);
	SafeRelease(m_pNavigationManager);
	SafeRelease(m_pPickingManager);

	SafeRelease(m_pIMGUIManager);
	SafeRelease(m_pGridManager);
	SafeRelease(m_pFontManager);
	
	SafeRelease(m_pLightManager);
	SafeRelease(m_pPipeLine);

	SafeRelease(m_pSceneManager);
	SafeRelease(m_pRenderManager);
	SafeRelease(m_pObjectManager);
	SafeRelease(m_pPrototypeManager);

	SafeRelease(m_pTimeManager);
	SafeRelease(m_pRenderTargetManager);
	SafeRelease(m_pInput);

	SafeRelease(m_pGraphic);

	DestroyInstance();
}

void EngineUtility::Free()
{
	__super::Free();
}

_float EngineUtility::Random(_float fMin, _float fMax)
{
	_float		fRandomNormal = static_cast<_float>(rand()) / RAND_MAX;
	return fMin + fRandomNormal * (fMax - fMin);
}

void EngineUtility::SetPathToBin()
{
	std::filesystem::path cur = std::filesystem::current_path();
	while (cur.has_parent_path() && cur.filename() != "bin")
		cur = cur.parent_path();
	if (cur.filename() == "bin")
		std::filesystem::current_path(cur);
}

ID3D11Device* EngineUtility::GetDevice()
{
	return m_pGraphic->GetDevice();
}

ID3D11DeviceContext* EngineUtility::GetContext()
{
	return m_pGraphic->GetContext();
}

HWND EngineUtility::GetWindowHandle()
{
	return m_pGraphic->GetWindowHandle();
}

_float2 EngineUtility::GetWindowSize()
{
	return m_pGraphic->GetWindowSize();
}

void EngineUtility::SnapDepthForPicking()
{
	m_pGraphic->SnapDepthForPicking();
}

_bool EngineUtility::ReadDepthAtPixel(_int px, _int py, _float* outDepth01)
{
	return m_pGraphic->ReadDepthAtPixel(px, py, outDepth01);
}

_byte EngineUtility::GetKeyState(_ubyte byKeyID)
{
	return m_pInput->GetKeyState(byKeyID);
}

_byte EngineUtility::GetMouseState(MOUSEKEYSTATE eMouse)
{
	return m_pInput->GetMouseState(eMouse);
}

_long EngineUtility::GetMouseMove(MOUSEMOVESTATE eMouseState)
{
	return m_pInput->GetMouseMove(eMouseState);
}

_float2 EngineUtility::GetMousePos()
{
	return m_pInput->GetMousePos();
}

void EngineUtility::SetMousePos(_float2 mousePos)
{
	m_pInput->SetMousePos(mousePos);
}

void EngineUtility::SetMouseVisible(_bool bVisible)
{
	m_pInput->SetMouseVisible(bVisible);
}

_bool EngineUtility::IsKeyDown(_ubyte byKeyID) const
{
	return m_pInput->IsKeyDown(byKeyID);
}

_bool EngineUtility::IsKeyPressed(_ubyte byKeyID) const
{
	return m_pInput->IsKeyPressed(byKeyID);
}

_bool EngineUtility::IsKeyReleased(_ubyte byKeyID) const
{
	return m_pInput->IsKeyReleased(byKeyID);
}

_bool EngineUtility::IsKeyUp(_ubyte byKeyID) const
{
	return m_pInput->IsKeyUp(byKeyID);
}

_bool EngineUtility::IsMouseDown(MOUSEKEYSTATE eMouse) const
{
	return m_pInput->IsMouseDown(eMouse);
}

_bool EngineUtility::IsMousePressed(MOUSEKEYSTATE eMouse) const
{
	return m_pInput->IsMousePressed(eMouse);
}

_bool EngineUtility::IsMouseReleased(MOUSEKEYSTATE eMouse) const
{
	return m_pInput->IsMouseReleased(eMouse);
}

_bool EngineUtility::IsMouseUp(MOUSEKEYSTATE eMouse) const
{
	return m_pInput->IsMouseUp(eMouse);
}

_float EngineUtility::GetTimeDelta(const _wstring& pTimerTag)
{
	return m_pTimeManager->GetTimeDelta(pTimerTag);
}

HRESULT EngineUtility::AddTimer(const _wstring& pTimerTag)
{
	return m_pTimeManager->AddTimer(pTimerTag);
}

void EngineUtility::UpdateTimeDelta(const _wstring& pTimerTag)
{
	m_pTimeManager->UpdateTimeDelta(pTimerTag);
}

HRESULT EngineUtility::ChangeScene(_uint iSceneId, Scene* pScene)
{
	return m_pSceneManager->ChangeScene(iSceneId, pScene);
}

void EngineUtility::ClearScene(_uint iSceneId)
{
	m_pObjectManager->Clear(iSceneId);
	m_pPrototypeManager->Clear(iSceneId);
}

_uint EngineUtility::GetCurrentSceneId()
{
	return m_pSceneManager->GetCurrentSceneId();
}

HRESULT EngineUtility::AddPrototype(_uint iSceneId, const _wstring& strPrototypeTag, Base* pPrototype)
{
	return m_pPrototypeManager->AddPrototype(iSceneId, strPrototypeTag, pPrototype);;
}

Base* EngineUtility::ClonePrototype(PROTOTYPE eType, _uint iSceneId, const _wstring& strPrototypeTag, void* pArg)
{
	return m_pPrototypeManager->ClonePrototype(eType, iSceneId, strPrototypeTag, pArg);
}

_bool EngineUtility::HasPrototype(_uint iSceneId, const _wstring& strPrototypeTag)
{
	return m_pPrototypeManager->HasPrototype(iSceneId, strPrototypeTag);
}

HRESULT EngineUtility::AddObject(_uint iPrototypeSceneId, const _wstring& strPrototypeTag, _uint iLayerSceneId, const _wstring& strLayerTag, void* pArg) 
{
	return m_pObjectManager->AddObject(iPrototypeSceneId, strPrototypeTag, iLayerSceneId, strLayerTag, pArg);
}

Object* EngineUtility::FindObject(_uint iLayerSceneId, const _wstring& strLayerTag, _uint iIndex)
{
	return m_pObjectManager->FindObject(iLayerSceneId, strLayerTag, iIndex);
}

Layer* EngineUtility::FindLayer(_uint iSceneId, const _wstring& strLayerTag)
{
	return m_pObjectManager->FindLayer(iSceneId,strLayerTag);
}

_uint EngineUtility::GetLayerSize(_uint iSceneId, const _wstring& strLayerTag)
{
	return m_pObjectManager->GetLayerSize(iSceneId, strLayerTag);
}

std::vector<class Object*> EngineUtility::GetAllObjects(_uint iSceneId)
{
	return m_pObjectManager->GetAllObjects(iSceneId);
}

void EngineUtility::ClearDeadObjects()
{
	m_pObjectManager->ClearDeadObjects();
}

HRESULT EngineUtility::JoinRenderGroup(RENDERGROUP eGroupID, class Object* pObject)
{
	return m_pRenderManager->JoinRenderGroup(eGroupID, pObject);
}

#ifdef _DEBUG
void EngineUtility::RenderDebug()
{
	return m_pRenderManager->RenderDebug();
}
#endif

const _float4x4* EngineUtility::GetTransformFloat4x4Ptr(D3DTS eState)
{

	return m_pPipeLine->GetTransformFloat4x4Ptr(eState);
}

_matrix EngineUtility::GetTransformMatrix(D3DTS eState)
{
	return m_pPipeLine->GetTransformMatrix(eState);
}

const _float4x4* EngineUtility::GetTransformFloat4x4InversePtr(D3DTS eState)
{
	return m_pPipeLine->GetTransformFloat4x4InversePtr(eState);
}

_matrix EngineUtility::GetTransformMatrixInverse(D3DTS eState)
{
	return m_pPipeLine->GetTransformMatrixInverse(eState);
}

const _float4* EngineUtility::GetCamPosition()
{
	return m_pPipeLine->GetCamPosition();
}

void EngineUtility::SetPipelineTransform(D3DTS eState, _fmatrix TransformMatrix)
{
	m_pPipeLine->SetPipelineTransform(eState, TransformMatrix);
}

const _float* EngineUtility::GetPipelineFarDistance()
{
	return m_pPipeLine->GetPipelineFarDistance();
}

void EngineUtility::SetPipelineFarDistance(const _float& fFarDistance)
{
	m_pPipeLine->SetPipelineFarDistance(fFarDistance);
}

const LIGHT_DESC* EngineUtility::GetLight(_uint iIndex)
{
	return m_pLightManager->GetLight(iIndex);
}

HRESULT EngineUtility::AddLight(const LIGHT_DESC& LightDesc)
{
	return m_pLightManager->AddLight(LightDesc);
}

HRESULT EngineUtility::RemoveLight(_uint iIndex)
{
	return m_pLightManager->RemoveLight(iIndex);
}

const list<class Light*>& EngineUtility::GetAllLights()
{
	return m_pLightManager->GetAllLights();
}

const list<class Light*>& EngineUtility::GetActiveLights()
{
	return m_pLightManager->GetActiveLights();
}

void EngineUtility::ClearLights()
{
	m_pLightManager->ClearLights();
}

HRESULT EngineUtility::RenderLights(class Shader* pShader, class VIBufferRect* pVIBuffer)
{
	return m_pLightManager->RenderLights(pShader, pVIBuffer);
}

void EngineUtility::SetLightActive(_uint iIndex, _bool bActive)
{
	return m_pLightManager->SetLightActive(iIndex, bActive);
}

void EngineUtility::SetLightActive(Light* pLight, _bool bActive)
{
	return m_pLightManager->SetLightActive(pLight, bActive);
}

void EngineUtility::SetActiveLightsByDistance(_fvector vPos, _float fMaxDistance, _uint iMaxLights)
{
	return m_pLightManager->SetActiveLightsByDistance(vPos, fMaxDistance, iMaxLights);
}

HRESULT EngineUtility::AddFont(const _wstring& strFontTag, const _tchar* pFontFilePath)
{
	return m_pFontManager->AddFont(strFontTag, pFontFilePath);
}

HRESULT EngineUtility::DrawFont(const _wstring& strFontTag, const _wstring& strText, const _float2& vPosition, _fvector vColor)
{
	return m_pFontManager->DrawFont(strFontTag, strText, vPosition, vColor);
}

#ifdef _IMGUI
HRESULT EngineUtility::BeginIMGUI()
{
	if (!m_pIMGUIManager)
		return E_FAIL;
	return m_pIMGUIManager->Begin();
}

HRESULT EngineUtility::RenderIMGUI()
{
	return m_pIMGUIManager->Render();
}

HRESULT EngineUtility::ShutdownIMGUI()
{
	return m_pIMGUIManager->Shutdown();
}

HRESULT EngineUtility::AddPanel(const string& PanelName, class Panel* pPanel) 
{
	return m_pIMGUIManager->AddPanel(PanelName, pPanel);
}

HRESULT EngineUtility::RemovePanel(const string& PanelName)
{
	return m_pIMGUIManager->RemovePanel(PanelName);
}

Panel* EngineUtility::FindPanel(const string& PanelName)
{
	return m_pIMGUIManager->FindPanel(PanelName);
}

HRESULT EngineUtility::ClearPanels()
{
	return m_pIMGUIManager->ClearPanels();
}

HRESULT EngineUtility::SetPanelOpen(const string& PanelName, bool open)
{
	return m_pIMGUIManager->SetPanelOpen(PanelName, open);
}

ImGuiContext* EngineUtility::GetIMGUIContext()
{
	return m_pIMGUIManager->GetIMGUIContext();
}

void EngineUtility::DrawPanels()
{
	m_pIMGUIManager->DrawPanels();
}

void EngineUtility::SetGizmoState(Object* pTarget, ImGuizmo::OPERATION eOperation)
{
	m_pIMGUIManager->SetGizmoTarget(pTarget, eOperation);
}

std::pair<class Object*, ImGuizmo::OPERATION> EngineUtility::GetGizmoState() const
{
	return m_pIMGUIManager->GetGizmoTarget();
}

bool EngineUtility::HasGizmoTarget() const
{
	return m_pIMGUIManager->HasGizmoTarget();
}

void EngineUtility::ClearGizmoState()
{
	m_pIMGUIManager->ClearGizmoTarget();
}
#endif

RAY EngineUtility::GetRay()
{
	return m_pPickingManager->GetRay();
}

PICK_RESULT EngineUtility::Pick()
{
	return m_pPickingManager->Pick();
}

_bool EngineUtility::RayIntersectObject(const RAY& ray, Object* pObject)
{
	return m_pPickingManager->RayIntersectObject(ray, pObject);
}

_bool EngineUtility::RayIntersectTerrain(const RAY& ray, Terrain* pTerrain)
{
	return m_pPickingManager->RayIntersectTerrain(ray, pTerrain);
}

#ifdef _DEBUG
void EngineUtility::RenderGrid()
{
	if(m_pGridManager->IsVisible())
		m_pGridManager->Render();
}

void EngineUtility::SetGridVisible(_bool enable)
{
	m_pGridManager->SetVisible(enable);
}

_bool EngineUtility::IsGridVisible() const
{
	return m_pGridManager->IsVisible();
}
#endif

_float EngineUtility::GetGridCellSize()
{
	return m_pGridManager->GetGridCellSize();
}

void EngineUtility::SetGridCellSize(_float cellSize)
{
	m_pGridManager->SetGridCellSize(cellSize);
}

_uint EngineUtility::GetNumGridCells()
{
	return m_pGridManager->GetNumGridCells();
}

void EngineUtility::SetNumGridCells(_uint iNumGridCells)
{
	m_pGridManager->SetNumGridCells(iNumGridCells);
}

void EngineUtility:: SetMarkerPosition(const _float3& vPos)
{
	m_pGridManager->SetMarkerPosition(vPos);
}

void EngineUtility::ClearMarker()
{
	m_pGridManager->ClearMarker();
}

_float3 EngineUtility::GetMarkerPosition() const
{
	return m_pGridManager->GetMarkerPosition();
}

_bool EngineUtility::IsMark() const
{
	return m_pGridManager->IsMark();
}

#ifdef _DEBUG
HRESULT EngineUtility::RenderNavigation()
{
	return m_pNavigationManager->Render();
}
#endif

void EngineUtility::AddTempPoint(const _float3& point)
{
	m_pNavigationManager->AddTempPoint(point);
}

void EngineUtility::ClearTempPoints()
{
	m_pNavigationManager->ClearTempPoints();
}

void EngineUtility::RemoveCell(_int cellIndex)
{
	m_pNavigationManager->RemoveCell(cellIndex);
}

void EngineUtility::RemoveRecentCell()
{
	m_pNavigationManager->RemoveRecentCell();
}

void EngineUtility::ClearCells()
{
	m_pNavigationManager->ClearCells();
}

void EngineUtility::SaveCells(const _char* pNavigationDataFile)
{
	m_pNavigationManager->SaveCells(pNavigationDataFile);
}

void EngineUtility::LoadCells(const _char* pNavigationDataFilePath)
{
	m_pNavigationManager->LoadCells(pNavigationDataFilePath);
}

_bool EngineUtility::IsInCell(_fvector vWorldPos, _int* pOutCellIndex)
{
	return m_pNavigationManager->IsInCell(vWorldPos, pOutCellIndex);
}

_float EngineUtility::GetHeightPosOnCell(_vector* pPos, const _int& pCellIndex)
{
	return m_pNavigationManager->GetHeightPosOnCell(pPos, pCellIndex);
}

_bool EngineUtility::SetHeightOnCell(_fvector vWorldPos, _vector* pOutAdjustedPos)
{
	return m_pNavigationManager->SetHeightOnCell(vWorldPos, pOutAdjustedPos);
}

_bool EngineUtility::GetSlideVectorOnCell(_fvector pos, _fvector delta, _int cellIndex, _vector* outSlideVector) const
{
	return m_pNavigationManager->GetSlideVectorOnCell(pos, delta, cellIndex, outSlideVector);
}

const vector<class Cell*>& EngineUtility::GetCells() const
{
	return m_pNavigationManager->GetCells();
}

_bool EngineUtility::Edit_AddTriangleOnEdge(_int cellId, _fvector pickedPoint, _float weldEps)
{
	return m_pNavigationManager->Edit_AddTriangleOnEdge(cellId, pickedPoint, weldEps);
}

_bool EngineUtility::Edit_AddTriangleAtSharedVertex(_int cellA, _int cellB, _float weldEps)
{
	return m_pNavigationManager->Edit_AddTriangleAtSharedVertex(cellA, cellB, weldEps);
}

_bool EngineUtility::RandomPointAround(_fvector center, _float radius, _float3* outPos, _uint maxTrials)
{
	return m_pNavigationManager->RandomPointAround(center, radius, outPos, maxTrials);
}

ModelData* EngineUtility::LoadNoAssimpModel(const _char* pFilePath)
{
	return m_pSaveLoadManager->LoadNoAssimpModel(pFilePath);
}

std::vector<MAP_OBJECTDATA> EngineUtility::LoadMapData(const std::string& path)
{
	return m_pSaveLoadManager->LoadMapData(path);
}

HRESULT EngineUtility::SaveMapData(const std::string& path)
{
	return m_pSaveLoadManager->SaveMapData(path);
}

HRESULT EngineUtility::SaveLights(const std::string& path)
{
	return m_pSaveLoadManager->SaveLights(path);
}

HRESULT EngineUtility::ReadyLightsFromFile(const std::string& path)
{
	return m_pSaveLoadManager->ReadyLightsFromFile(path);
}

HRESULT EngineUtility::SaveTriggerBoxes(const std::string& path)
{
	return m_pSaveLoadManager->SaveTriggerBoxes(path);
}

HRESULT EngineUtility::LoadTriggerBoxes(const std::string& path)
{
	return m_pSaveLoadManager->LoadTriggerBoxes(path);
}
HRESULT EngineUtility::BuildUIFromRes(const std::string& path, const UIPrototypeTags& protoTags, std::vector<class UI*>& outUIObjects)
{
	return m_pSaveLoadManager->BuildUIFromRes(path, protoTags, outUIObjects);
}

HRESULT EngineUtility::AddRenderTarget(const _wstring& strRenderTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor)
{
	return m_pRenderTargetManager->AddRenderTarget(strRenderTargetTag, iWidth, iHeight, ePixelFormat, vClearColor);
}

HRESULT EngineUtility::AddRenderTargetGroup(const _wstring& strRenderTargetGroupTag, const _wstring& strRenderTargetTag)
{
	return m_pRenderTargetManager->AddRenderTargetGroup(strRenderTargetGroupTag, strRenderTargetTag);
}

HRESULT EngineUtility::BeginRenderTargetGroup(const _wstring& strRenderTargetGroupTag, _bool isClearDepth, ID3D11DepthStencilView* pDepthStencilView)
{
	return m_pRenderTargetManager->BeginRenderTargetGroup(strRenderTargetGroupTag, isClearDepth, pDepthStencilView);
}

HRESULT EngineUtility::EndRenderTargetGroup()
{
	return m_pRenderTargetManager->EndRenderTargetGroup();
}

HRESULT EngineUtility::BindRenderTargetShaderResource(const _wstring& strRenderTargetTag, class Shader* pShader, const _char* pConstantName)
{
	return m_pRenderTargetManager->BindRenderTargetShaderResource(strRenderTargetTag, pShader, pConstantName);
}

HRESULT EngineUtility::CopyRenderTargetResource(const _wstring& strRenderTargetTag, ID3D11Texture2D* pOut)
{
	return m_pRenderTargetManager->CopyRenderTargetResource(strRenderTargetTag, pOut);
}

#ifdef _DEBUG
HRESULT EngineUtility::ReadyRenderTargetDebug(const _wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	return m_pRenderTargetManager->ReadyRenderTargetDebug(strRenderTargetTag, fX, fY, fSizeX, fSizeY);
}

HRESULT EngineUtility::RenderDebugRenderTargetGroup(const _wstring& strRenderTargetGroupTag, class Shader* pShader, class VIBufferRect* pVIBuffer)
{
	return m_pRenderTargetManager->RenderDebugRenderTargetGroup(strRenderTargetGroupTag, pShader, pVIBuffer);
}
#endif

const SHADOW_DESC* EngineUtility::GetShadowLight(_uint iIndex)
{
	return m_pShadowLightManager->GetShadowLight(iIndex);
}

HRESULT EngineUtility::AddShadowLight(const SHADOW_DESC& ShadowDesc)
{
	return m_pShadowLightManager->AddShadowLight(ShadowDesc);
}

HRESULT EngineUtility::RemoveShadowLight(_uint iIndex)
{
	return m_pShadowLightManager->RemoveShadowLight(iIndex);
}

const list<class ShadowLight*>& EngineUtility::GetAllShadowLights()
{
	return m_pShadowLightManager->GetAllShadowLights();
}

const list<class ShadowLight*>& EngineUtility::GetActiveShadowLights()
{
	return m_pShadowLightManager->GetActiveShadowLights();
}

void EngineUtility::ClearShadowLights()
{
	return m_pShadowLightManager->ClearShadowLights();
}

const _float4x4* EngineUtility::GetActiveShadowLightTransformFloat4x4Ptr(D3DTS eState, _uint iIndex)
{
	return m_pShadowLightManager->GetActiveShadowLightTransformFloat4x4Ptr(eState, iIndex);
}

const _float3* EngineUtility::GetActiveShadowLightPositionPtr(_uint iIndex)
{
	return m_pShadowLightManager->GetActiveShadowLightPositionPtr(iIndex);
}

const _float* EngineUtility::GetActiveShadowLightFarDistancePtr(_uint iIndex)
{
	return m_pShadowLightManager->GetActiveShadowLightFarDistancePtr(iIndex);
}

_uint EngineUtility::GetNumActiveShadowLights()
{
	return m_pShadowLightManager->GetNumActiveShadowLights();
}

void EngineUtility::SetShadowLightActive(_uint iIndex, _bool bActive)
{
	return m_pShadowLightManager->SetShadowLightActive(iIndex, bActive);
}

void EngineUtility::SetShadowLightActive(class ShadowLight* pShadowLight, _bool bActive)
{
	return m_pShadowLightManager->SetShadowLightActive(pShadowLight, bActive);
}

void EngineUtility::SetActiveShadowLightsByDistance(_fvector vPos, _float fMaxDistance, _uint iMaxCount)
{
	return m_pShadowLightManager->SetActiveShadowLightsByDistance(vPos, fMaxDistance, iMaxCount);
}

void EngineUtility::AddSpawner(class Spawner* pSpawner)
{
	m_pSpawnerManager->AddSpawner(pSpawner);
}

void EngineUtility::Spawn(_uint iSpawnerIndex)
{
	m_pSpawnerManager->Spawn(iSpawnerIndex);
}

void EngineUtility::RemoveSpawner(_uint iSpawnerIndex)
{
	m_pSpawnerManager->RemoveSpawner(iSpawnerIndex);
}

void EngineUtility::ClearSpawners()
{
	m_pSpawnerManager->ClearSpawners();
}

const vector<Spawner*>& EngineUtility::GetSpawners() const
{
	return m_pSpawnerManager->GetSpawners();
}

HRESULT EngineUtility::AddTriggerBox(class TriggerBox* pTriggerBox)
{
	return m_pTriggerBoxManager->AddTriggerBox(pTriggerBox);
}

HRESULT EngineUtility::RemoveTriggerBox(_uint index)
{
	return m_pTriggerBoxManager->RemoveTriggerBox(index);
}

void EngineUtility::ClearTriggerBoxes()
{
	m_pTriggerBoxManager->ClearTriggerBoxes();
}

void EngineUtility::UpdateTriggers()
{
	m_pTriggerBoxManager->UpdateTriggers();
}

void EngineUtility::RenderTriggerBoxes()
{
	m_pTriggerBoxManager->RenderTriggerBoxes();
}

const vector<class TriggerBox*>& EngineUtility::GetTriggerBoxes() const
{
	return m_pTriggerBoxManager->GetTriggerBoxes();
}

_bool EngineUtility::IsIn_Frustum_WorldSpace(_fvector vWorldPos, _float fRadius)
{
	return m_pFrustum->IsIn_WorldSpace(vWorldPos, fRadius);
}
