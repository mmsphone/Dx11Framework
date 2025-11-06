#pragma once

#include "Client_Defines.h"
#include "Camera.h"

NS_BEGIN(Client)

class ChaseCam final : public Camera
{
public:
    typedef struct tagCamera_Chase_Desc : public Camera::CAMERA_DESC
    {
        class Object* pTarget = nullptr;
        _vector offset = {};
    } CHASECAM_DESC;

private:
    ChaseCam();
    ChaseCam(const ChaseCam& Prototype);
    virtual ~ChaseCam() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void PriorityUpdate(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static ChaseCam* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void Free() override;

private:
    class Object* m_pTarget = nullptr;
    _vector m_vOffset = {};
};

NS_END
