#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class PickingManager final : public Base
{
    PickingManager();
    virtual ~PickingManager() = default;

public:
    PICK_RESULT Pick();

    _bool IsMouseOverUI() const;
    _bool RayIntersectObject(const RAY& ray, class Object* pObject, _float3* pOutHitPos = nullptr) const;
    _bool RayIntersectTerrain(const RAY& ray, class Terrain* pTerrain, _float3* pOutHitPos = nullptr) const;

    RAY GetRay();

    static PickingManager* Create();
    virtual void Free() override;
    
private:
    _float3 Unproject(_float depth01, _float mouseX, _float mouseY, _float winW, _float winH, const _float4x4& invViewProj);


private:
    class EngineUtility* m_pEngineUtility = nullptr;
};

NS_END
