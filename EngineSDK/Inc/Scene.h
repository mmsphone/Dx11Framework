#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL Scene abstract : public Base
{
protected:
	Scene();
	virtual ~Scene() = default;

public:
	virtual HRESULT Initialize();
	virtual void Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void Free() override;

protected:
	class EngineUtility* m_pEngineUtility = nullptr;
};

NS_END