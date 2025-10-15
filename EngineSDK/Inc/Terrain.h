#pragma once

#include "Object.h"

NS_BEGIN(Engine)

class VIBufferTerrain;
class Shader;
class Texture;

class ENGINE_DLL Terrain abstract : public Object
{
protected:
	Terrain();
	Terrain(const Terrain& Prototype);
	virtual ~Terrain() = default;

public:
	virtual HRESULT InitializePrototype();
	virtual HRESULT Initialize(void* pArg);

	virtual void PriorityUpdate(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void LateUpdate(_float fTimeDelta);

	virtual HRESULT Render();

	virtual Object* Clone(void* pArg) = 0;
	virtual void Free();
};

NS_END