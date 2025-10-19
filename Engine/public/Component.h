#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL Component abstract : public Base
{
protected:
	Component();
	Component(const Component& Prototype);
	virtual ~Component() = default;

public:
	virtual HRESULT InitializePrototype();
	virtual HRESULT Initialize(void* pArg);

	virtual Component* Clone(void* pArg) = 0;
	virtual void Free() override;

protected:
	class EngineUtility* m_pEngineUtility = nullptr;
	_bool m_isCloned = false;
};

NS_END