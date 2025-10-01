#pragma once

#include "Client_Defines.h"
#include "Part.h"

NS_BEGIN(Engine)
class Shader;
class Model;
NS_END

NS_BEGIN(Client)

class PlayerWeapon final : public Part
{
public:
	typedef struct tagWeaponDesc final : public Part::PART_DESC
	{
		const _float4x4* pSocketBoneMatrix = { nullptr };
		const _uint* pParentState = { nullptr };
	}PLAYERWEAPON_DESC;
private:
	PlayerWeapon();
	PlayerWeapon(const PlayerWeapon& Prototype);
	virtual ~PlayerWeapon() = default;

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
	HRESULT BindShaderResources();
private:
	const _uint* m_pParentState = { nullptr };
	const _float4x4* m_pSocketBoneMatrix = { };	
};

NS_END