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

	_float Random(_float fMin, _float fMax);

	//Graphic
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetContext();

	//Input
	_byte	GetKeyState(_ubyte byKeyID);
	_byte	GetMouseState(MOUSEKEYSTATE eMouse);
	_long	GetMouseMove(MOUSEMOVESTATE eMouseState);

	//TimeManager
	_float GetTimeDelta(const _wstring& pTimerTag);
	HRESULT AddTimer(const _wstring& pTimerTag);
	void UpdateTimeDelta(const _wstring& pTimerTag);

	//SceneManager
	HRESULT ChangeScene(_uint iSceneId, class Scene* pScene);
	void ClearScene(_uint iSceneId);

	//PrototypeManager
	HRESULT AddPrototype(_uint iSceneId, const _wstring& strPrototypeTag, Base* pPrototype);
	Base* ClonePrototype(PROTOTYPE eType, _uint iSceneId, const _wstring& strPrototypeTag, void* pArg = nullptr);

	//ObjectManager
	HRESULT AddObject(_uint iPrototypeSceneId, const _wstring& strPrototypeTag, _uint iLayerSceneId, const _wstring& strLayerTag, void* pArg = nullptr);
	
	//RenderManager
	HRESULT JoinRenderGroup(RENDERGROUP eGroupID, class Object* pObject);

	//Pipeline
	const _float4x4* GetTransformFloat4x4Ptr(D3DTS eState);
	_matrix GetTransformMatrix(D3DTS eState);
	const _float4x4* GetTransformFloat4x4InversePtr(D3DTS eState);
	_matrix GetTransformMatrixInverse(D3DTS eState);
	const _float4* GetCamPosition();
	void SetTransform(D3DTS eState, _fmatrix TransformMatrix);

	//LightManager
	const LIGHT_DESC* GetLight(_uint iIndex);
	HRESULT AddLight(const LIGHT_DESC& LightDesc);

	//FontManger
	HRESULT AddFont(const _wstring& strFontTag, const _tchar* pFontFilePath);
	HRESULT DrawFont(const _wstring& strFontTag, const _wstring& strText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f));

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
};

NS_END