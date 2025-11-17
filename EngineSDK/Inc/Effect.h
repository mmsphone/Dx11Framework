#pragma once

#include "Object.h"

NS_BEGIN(Engine)

class ENGINE_DLL Effect abstract : public Object
{
public:
    typedef struct tagEffectDesc : public OBJECT_DESC
    {
        _float fLifeTime = 1.f;
        _bool  bLoop = false;
        _bool  bAutoKill = true;
    }EFFECT_DESC;

protected:
	Effect();
	Effect(const Effect& Prototype);
	virtual ~Effect() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void LateUpdate(_float fTimeDelta) override;

    const EFFECT_DESC& GetEffectDesc() const;
    _float             GetAccTime()   const;
    _float             GetLifeRatio() const;
    _bool              IsFinished()   const;

    virtual Object* Clone(void* pArg) = 0;
    virtual void    Free() override;

protected:
    EFFECT_DESC m_EffectDesc{};
    _float      m_fAccTime = 0.f;
    _bool       m_bFinished = false;
};

NS_END