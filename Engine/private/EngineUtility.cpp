﻿#include "EngineUtility.h"

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

	return S_OK;
}

void EngineUtility::UpdateEngine(_float fTimeDelta)
{
	m_pInput->Update();

	m_pObjectManager->PriorityUpdate(fTimeDelta);
	m_pPipeLine->Update();
	m_pObjectManager->Update(fTimeDelta);
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
	SafeRelease(m_pFontManager);
	SafeRelease(m_pLightManager);
	SafeRelease(m_pPipeLine);
	SafeRelease(m_pRenderManager);
	SafeRelease(m_pObjectManager);
	SafeRelease(m_pPrototypeManager);
	SafeRelease(m_pSceneManager);
	SafeRelease(m_pInput);
	SafeRelease(m_pGraphic);
	SafeRelease(m_pTimeManager);
	SafeRelease(m_pIMGUIManager);
	SafeRelease(m_pPickingManager);
	SafeRelease(m_pGridManager);
	SafeRelease(m_pNavigationManager);

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

void EngineUtility::SetTransform(D3DTS eState, _fmatrix TransformMatrix)
{
	m_pPipeLine->SetTransform(eState, TransformMatrix);
}

const LIGHT_DESC* EngineUtility::GetLight(_uint iIndex)
{
	return m_pLightManager->GetLight(iIndex);
}

HRESULT EngineUtility::AddLight(const LIGHT_DESC& LightDesc)
{
	return m_pLightManager->AddLight(LightDesc);
}

HRESULT EngineUtility::AddFont(const _wstring& strFontTag, const _tchar* pFontFilePath)
{
	return m_pFontManager->AddFont(strFontTag, pFontFilePath);
}

HRESULT EngineUtility::DrawFont(const _wstring& strFontTag, const _wstring& strText, const _float2& vPosition, _fvector vColor)
{
	return m_pFontManager->DrawFont(strFontTag, strText, vPosition, vColor);
}

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

void EngineUtility::RenderGrid()
{
#ifdef _DEBUG
	if(m_pGridManager->IsVisible())
		m_pGridManager->Render();
#endif
}

void EngineUtility::SetGridVisible(_bool enable)
{
	m_pGridManager->SetVisible(enable);
}

_bool EngineUtility::IsGridVisible() const
{
	return m_pGridManager->IsVisible();
}
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
void EngineUtility::AddCell(_float3* pPoints)
{
	m_pNavigationManager->AddCell(pPoints);
}
void EngineUtility::AddTempPoint(const _float3& point)
{
	m_pNavigationManager->AddTempPoint(point);
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
void EngineUtility::LoadCells(const _char* pNavigationDataFile)
{
	m_pNavigationManager->LoadCells(pNavigationDataFile);
}
_bool EngineUtility::IsInCell(_fvector vWorldPos, _int* pOutCellIndex)
{
	return m_pNavigationManager->IsInCell(vWorldPos, pOutCellIndex);
}
_bool EngineUtility::SetHeightOnCell(_fvector vWorldPos, _vector* pOutAdjustedPos)
{
	return m_pNavigationManager->SetHeightOnCell(vWorldPos, pOutAdjustedPos);
}
const vector<class Cell*>& EngineUtility::GetCells() const
{
	return m_pNavigationManager->GetCells();
}