#pragma once

#include "Base.h"

NS_(Engine)

/* �� ������ �׷����� �پ��� ��ü ����*/
/* �׷��� �ϴ� ������� (��κ� ��ü�� ���� �׽�Ʈ�� ���� ��� X, UI�� ����� ������ ��� O)*/
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
	list<class Object*> m_RenderObjects[RENDERGROUP::END];
	class EngineUtility* m_pEngineUtility = nullptr;
};

_NS