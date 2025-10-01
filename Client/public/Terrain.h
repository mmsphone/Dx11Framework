#pragma once

#include "Client_Defines.h"
#include "Object.h"

NS_BEGIN(Engine)
class Shader;
class Texture;
class VIBufferTerrain;
NS_END

NS_BEGIN(Client)

class Terrain final : public Object
{
public:
	enum TEXTURETYPE : int { TEXTURE_DIFFUSE, TEXTURE_MASK, TEXTURE_BRUSH, TEXTURE_END };
private:
	Terrain();
	Terrain(const Terrain& Prototype);
	virtual ~Terrain() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static Object* Create();/* 원형생성 */
	virtual Object* Clone(void* pArg) override;/* 사본생성 */
	virtual void Free();
private:
	HRESULT ReadyComponents();
};

NS_END