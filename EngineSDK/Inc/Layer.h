#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class Layer final : public Base
{
private:
	Layer();
	virtual ~Layer() = default;

public:
	HRESULT AddObject(class Object* pObject);
	class Object* FindObject(_uint iIndex);
	_uint GetLayerSize() const;
	list<class Object*>& GetAllObjects();

	void PriorityUpdate(_float fTimeDelta);
	void Update(_float fTimeDelta);
	void LateUpdate(_float fTimeDelta);

	static Layer* Create();
	virtual void Free() override;

private:
	list<class Object*> m_Objects = {};
};

NS_END