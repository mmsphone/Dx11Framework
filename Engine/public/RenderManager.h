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

	static RenderManager* Create();
	virtual void Free() override;

private:
	void RenderPriority();
	void RenderNonBlend();
	void RenderBlend();
	void RenderUI();
private:
	list<class Object*> m_RenderObjects[RENDERGROUP::RENDERGROUP_END];
	class EngineUtility* m_pEngineUtility = nullptr;
};

NS_END