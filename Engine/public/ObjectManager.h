#pragma once

#include "Base.h"

NS_BEGIN(Engine)

/* 원형을 복제한 사본들을 보관 */
class ObjectManager final : public Base
{
private:
	ObjectManager();
	virtual ~ObjectManager() = default;

public:
	HRESULT Initialize(_uint iNumScenes);
	HRESULT AddObject(_uint iPrototypeSceneId, const _wstring& strPrototypeTag, _uint iLayerSceneId, const _wstring& strLayerTag, void* pArg);
	class Object* FindObject(_uint iLayerSceneId, const _wstring& strLayerTag, _uint iIndex);
	class Layer* FindLayer(_uint iSceneId, const _wstring& strLayerTag);
	_uint GetLayerSize(_uint iSceneId, const _wstring& strLayerTag);

	void Clear(_uint iSceneId);
	void PriorityUpdate(_float fTimeDelta);
	void Update(_float fTimeDelta);
	void LateUpdate(_float fTimeDelta);

	static ObjectManager* Create(_uint iNumScenes);
	virtual void Free() override;

private:
	_uint m_iNumScenes = {};
	map<const _wstring, class Layer*>* m_pLayers = { nullptr };
	class EngineUtility* m_pEngineUtility;
};

NS_END