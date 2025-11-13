#pragma once

#include "Base.h"

NS_BEGIN(Engine)

/* 씬 내에서 그려지는 다양한 객체 모음*/
/* 그려야 하는 순서대로 (대부분 객체는 깊이 테스트로 순서 상관 X, UI나 배경은 순서가 상관 O)*/
/* Priority -> NonBlend -> Blend -> UI*/

class RenderManager final : public Base
{
private:
	RenderManager();
	virtual ~RenderManager() = default;

public:
	HRESULT Initialize();
	HRESULT JoinRenderGroup(RENDERGROUP eGroupId, class Object* pObject);
	void Draw();

#ifdef _DEBUG
	HRESULT AddDebugComponent(class Component* pDebugComponent);
#endif

	static RenderManager* Create();
	virtual void Free() override;

private:
	void RenderPriority();
	void RenderShadowLight();
	void RenderNonBlend(); 
	void RenderLights();
	void RenderCombined();
	void RenderNonLights();
	void RenderBlend();
	void RenderUI();

	HRESULT ReadyShadowDepthStencilView();
	HRESULT ChangeViewportSize(_uint iWidth, _uint iHeight);

#ifdef _DEBUG
	void Render();
#endif

private:
	class EngineUtility* m_pEngineUtility = { nullptr };

	list<class Object*> m_RenderObjects[RENDERGROUP::RENDERGROUP_END];
	_float4x4				m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	class VIBufferRect* m_pVIBuffer = { nullptr };
	class Shader* m_pShader = { nullptr };
	ID3D11DepthStencilView* m_pShadowDepthStencilView = { nullptr };

#ifdef _DEBUG
	list<class Component*>			m_DebugComponents;
#endif
};

NS_END