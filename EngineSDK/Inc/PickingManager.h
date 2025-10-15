#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class PickingManager final : public Base
{
    PickingManager();
    virtual ~PickingManager() = default;

public:
    _float3 GetRayHitPosition(const RAY& ray, class Object* pObject) const;

    _bool RayIntersectObject(const RAY& ray, class Object* pObject) const;
    _bool RayIntersectTerrain(const RAY& ray, class Terrain* pTerrain) const;

    RAY GetRay();

    static PickingManager* Create();
    virtual void Free() override;
    
private:
    class EngineUtility* m_pEngineUtility = nullptr;
};

NS_END
