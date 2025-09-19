#pragma once

#include "Base.h"

NS_(Engine)

/* �� ��ȯ�� ��,  ���� �� �ı��� �ڿ� ����*/
/* ���� �ʿ��� �� ����*/
/* �ݺ����� ������Ʈ, ���� ȣ��*/
class SceneManager final : public Base
{
private:
	SceneManager();
	virtual ~SceneManager() = default;

public:
	HRESULT ChangeScene(_uint iSceneId, class Scene* pScene);
	void Update(_float fTimeDelta);
	HRESULT Render();

	static SceneManager* Create();
	virtual void Free() override;

private:
	_uint m_iSceneId = {};
	class Scene* m_pCurrentScene = { nullptr };
	class EngineUtility* m_pEngineUtility = { nullptr };
};

_NS