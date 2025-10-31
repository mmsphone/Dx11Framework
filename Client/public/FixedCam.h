#pragma once

#include "Client_Defines.h"
#include "Camera.h"

NS_BEGIN(Client)

class FixedCam final : public Camera
{
public:
    typedef struct tagCamera_Fixed_Desc : public Camera::CAMERA_DESC
    {

    } FIXEDCAM_DESC;

private:
    FixedCam();
    FixedCam(const FixedCam& Prototype);
    virtual ~FixedCam() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void PriorityUpdate(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static FixedCam* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END
