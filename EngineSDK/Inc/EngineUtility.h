#pragma once

#include "Base.h"
#include "PrototypeManager.h"

NS_BEGIN(Engine)

class ENGINE_DLL EngineUtility final : public Base
{
	DECLARE_SINGLETON(EngineUtility)

private:
	EngineUtility();
	virtual ~EngineUtility() = default;

public:
	//Engine
	HRESULT InitializeEngine(const ENGINE_DESC& EngineDesc);
	void UpdateEngine(_float fTimeDelta);
	HRESULT BeginDraw(const _float4* pColor);
	HRESULT Draw();
	HRESULT EndDraw();
	void ReleaseEngine();
	virtual void Free() override;

	//Utility
	_float Random(_float fMin, _float fMax);
	void SetPathToBin();

	//Graphic
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetContext();
	HWND GetWindowHandle();
	_float2 GetWindowSize();
	void SnapDepthForPicking();
	_bool ReadDepthAtPixel(_int px, _int py, _float* outDepth01);

	//Input
	_byte	GetKeyState(_ubyte byKeyID);
	_byte	GetMouseState(MOUSEKEYSTATE eMouse);
	_long	GetMouseMove(MOUSEMOVESTATE eMouseState);
	_float2 GetMousePos();
	void SetMousePos(_float2 mousePos);
	void SetMouseVisible(_bool bVisible);
	_bool IsKeyDown(_ubyte byKeyID) const;
	_bool IsKeyPressed(_ubyte byKeyID) const;
	_bool IsKeyReleased(_ubyte byKeyID) const;
	_bool IsKeyUp(_ubyte byKeyID) const;
	_bool IsMouseDown(MOUSEKEYSTATE eMouse) const;
	_bool IsMousePressed(MOUSEKEYSTATE eMouse) const;
	_bool IsMouseReleased(MOUSEKEYSTATE eMouse) const;
	_bool IsMouseUp(MOUSEKEYSTATE eMouse) const;

	//TimeManager
	_float GetTimeDelta(const _wstring& pTimerTag);
	HRESULT AddTimer(const _wstring& pTimerTag);
	void UpdateTimeDelta(const _wstring& pTimerTag);

	//SceneManager
	HRESULT ChangeScene(_uint iSceneId, class Scene* pScene);
	void ClearScene(_uint iSceneId);
	_uint GetCurrentSceneId();

	//PrototypeManager
	HRESULT AddPrototype(_uint iSceneId, const _wstring& strPrototypeTag, Base* pPrototype);
	Base* ClonePrototype(PROTOTYPE eType, _uint iSceneId, const _wstring& strPrototypeTag, void* pArg = nullptr); 
	_bool HasPrototype(_uint iSceneId, const _wstring& strPrototypeTag);

	//ObjectManager
	HRESULT AddObject(_uint iPrototypeSceneId, const _wstring& strPrototypeTag, _uint iLayerSceneId, const _wstring& strLayerTag, void* pArg = nullptr);
	class Object* FindObject(_uint iLayerSceneId, const _wstring& strLayerTag, _uint iIndex);
	class Layer* FindLayer(_uint iSceneId, const _wstring& strLayerTag);
	_uint GetLayerSize(_uint iSceneId, const _wstring& strLayerTag);
	std::vector<class Object*> GetAllObjects(_uint iSceneId);
	void ClearDeadObjects();

	//RenderManager
	HRESULT JoinRenderGroup(RENDERGROUP eGroupID, class Object* pObject);

	//Pipeline
	const _float4x4* GetTransformFloat4x4Ptr(D3DTS eState);
	_matrix GetTransformMatrix(D3DTS eState);
	const _float4x4* GetTransformFloat4x4InversePtr(D3DTS eState);
	_matrix GetTransformMatrixInverse(D3DTS eState);
	const _float4* GetCamPosition();
	void SetPipelineTransform(D3DTS eState, _fmatrix TransformMatrix);
	const _float* GetPipelineFarDistance();
	void SetPipelineFarDistance(const _float& fFarDistance);

	//LightManager
	const LIGHT_DESC* GetLight(_uint iIndex);
	HRESULT AddLight(const LIGHT_DESC& LightDesc);
	HRESULT RenderLights(class Shader* pShader, class VIBufferRect* pVIBuffer);

	//FontManger
	HRESULT AddFont(const _wstring& strFontTag, const _tchar* pFontFilePath);
	HRESULT DrawFont(const _wstring& strFontTag, const _wstring& strText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f));

#ifdef _IMGUI
	//IMGUIManager
	HRESULT BeginIMGUI();
	HRESULT RenderIMGUI();
	HRESULT ShutdownIMGUI();
	HRESULT AddPanel(const string& PanelName, class Panel* pPanel);
	HRESULT RemovePanel(const string& PanelName);
	class Panel* FindPanel(const string& PanelName);
	HRESULT ClearPanels();
	HRESULT SetPanelOpen(const string& PanelName, bool open);
	ImGuiContext* GetIMGUIContext();
	void DrawPanels();
	void SetGizmoState(class Object* pTarget, ImGuizmo::OPERATION eOperation);
	std::pair<class Object*, ImGuizmo::OPERATION> GetGizmoState() const;
	bool HasGizmoTarget() const;
	void ClearGizmoState();
#endif

	//PickingManager
	RAY GetRay();
	PICK_RESULT Pick();
	_bool RayIntersectObject(const RAY& ray, class Object* pObject);
	_bool RayIntersectTerrain(const RAY& ray, class Terrain* pTerrain);

	//GridManager
#ifdef _DEBUG
	void RenderGrid();
	void SetGridVisible(_bool enable);
	_bool IsGridVisible() const;
#endif
	_float GetGridCellSize();
	void SetGridCellSize(_float cellSize);
	_uint GetNumGridCells();
	void SetNumGridCells(_uint iNumGridCells);
	void SetMarkerPosition(const _float3& vPos);
	void ClearMarker();
	_float3 GetMarkerPosition() const;
	_bool IsMark() const;

	//NavigationManager
#ifdef _DEBUG
	HRESULT RenderNavigation();
#endif
	void AddTempPoint(const _float3& point);
	void ClearTempPoints();
	void RemoveRecentCell();
	void ClearCells();
	void SaveCells(const _char* pNavigationDataFile);
	void LoadCells(const _char* pNavigationDataFile);
	_bool IsInCell(_fvector vWorldPos, _int* pOutCellIndex = nullptr);
	_bool SetHeightOnCell(_fvector vWorldPos, _vector* pOutAdjustedPos);
	const vector<class Cell*>& GetCells() const;
	_bool Edit_AddTriangleOnEdge(_int cellId, _fvector pickedPoint, _float weldEps);
	_bool Edit_AddTriangleAtSharedVertex(_int cellA, _int cellB, _float weldEps);
	_bool RandomPointAround(_fvector center, _float radius, _float3* outPos, _uint maxTrials = 64);

	//SaveLoadManager
	ModelData* LoadNoAssimpModel(const _char* pFilePath);
	std::vector<MAP_OBJECTDATA> LoadMapData(const std::string& path);
	HRESULT SaveMapData(const std::string& path);

	//RenderTargetManager
	HRESULT AddRenderTarget(const _wstring& strRenderTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT AddRenderTargetGroup(const _wstring& strRenderTargetGroupTag, const _wstring& strRenderTargetTag);
	HRESULT BeginRenderTargetGroup(const _wstring& strRenderTargetGroupTag, _bool isClearDepth = false, ID3D11DepthStencilView* pDepthStencilView = nullptr);
	HRESULT EndRenderTargetGroup();
	HRESULT BindRenderTargetShaderResource(const _wstring& strRenderTargetTag, class Shader* pShader, const _char* pConstantName);
	HRESULT CopyRenderTargetResource(const _wstring& strRenderTargetTag, ID3D11Texture2D* pOut);
#ifdef _DEBUG
	HRESULT ReadyRenderTargetDebug(const _wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT RenderRenderTargetGroup(const _wstring& strRenderTargetGroupTag, class Shader* pShader, class VIBufferRect* pVIBuffer);
#endif

	//ShadowLightManager
	HRESULT AddShadowLight(const SHADOW_DESC& ShadowDesc);
	const _float4x4* GetShadowTransformFloat4x4Ptr(D3DTS eState, _uint iIndex = 0);	
	const _float3* GetShadowLightPositionPtr(_uint iIndex);
	const _float* GetShadowLightFarDistancePtr(_uint iIndex);
	void ClearShadowLights();
	_uint GetNumShadowLights();

private:
	class Graphic* m_pGraphic = { nullptr };
	class Input* m_pInput = { nullptr };
	class TimeManager* m_pTimeManager = { nullptr };
	class SceneManager* m_pSceneManager = { nullptr };
	class PrototypeManager* m_pPrototypeManager = { nullptr };
	class ObjectManager* m_pObjectManager = { nullptr };
	class RenderManager* m_pRenderManager = { nullptr };
	class Pipeline* m_pPipeLine = { nullptr };
	class LightManager* m_pLightManager = { nullptr };
	class FontManager* m_pFontManager = { nullptr };
	class IMGUIManager* m_pIMGUIManager = { nullptr };
	class PickingManager* m_pPickingManager = { nullptr };
	class GridManager* m_pGridManager = { nullptr };
	class NavigationManager* m_pNavigationManager = { nullptr };
	class SaveLoadManager* m_pSaveLoadManager = { nullptr };
	class RenderTargetManager* m_pRenderTargetManager = { nullptr };
	class ShadowLightManager* m_pShadowLightManager = { nullptr };
};

NS_END