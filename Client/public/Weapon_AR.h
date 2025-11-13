#pragma once

#include "Client_Defines.h"
#include "PartTemplate.h"

NS_BEGIN(Client)

class Weapon_AR final : public PartTemplate
{
public:
	typedef struct tagWeaponARDesc final : public Part::PART_DESC
	{
		const _float4x4* pSocketBoneMatrix = { nullptr };
	}WEAPON_AR_DESC;

private:
	Weapon_AR();
	Weapon_AR(const Weapon_AR& Prototype);
	virtual ~Weapon_AR() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT RenderShadow(_uint iIndex) override;

	static Weapon_AR* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free() override;

private:
	HRESULT ReadyComponents() override;

private:
	const _float4x4* m_pSocketBoneMatrix = { };

};

NS_END