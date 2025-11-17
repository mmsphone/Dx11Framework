#include "Spawner.h"

#include "EngineUtility.h"
#include "Layer.h"
#include "Object.h"

Spawner::Spawner()
    :m_pEngineUtility{ EngineUtility::GetInstance() }
{
    SafeAddRef(m_pEngineUtility);
}

HRESULT Spawner::Initialize()
{
    ClearSpawner();

    return S_OK;
}

void Spawner::AddSpawnerMob(const _uint& iSceneId, const _wstring& prototypeKey, const _wstring& layerKey, _fvector pos)
{
    SPAWNER_MOB_DESC desc{};
    desc.iSceneId = iSceneId;
    desc.prototypeKey = prototypeKey;
    desc.layerKey = layerKey;
    XMStoreFloat3(&desc.position, pos);

    m_MobDescs.push_back(desc);
}

void Spawner::ClearSpawner()
{
    m_MobDescs.clear();
}

void Spawner::Spawn()
{
    for (const auto& mob : m_MobDescs)
    {
        m_pEngineUtility->AddObject(mob.iSceneId, mob.prototypeKey, mob.iSceneId, mob.layerKey);
        Object* pObject = m_pEngineUtility->FindLayer(mob.iSceneId, mob.layerKey)->GetAllObjects().back();
        static_cast<Transform*>(pObject->FindComponent(TEXT("Transform")))->SetState(MATRIXROW_POSITION, XMVectorSet(mob.position.x, mob.position.y, mob.position.z ,1.f));
    }
}

const vector<SPAWNER_MOB_DESC>& Spawner::GetMobDescs() const
{
    return m_MobDescs;
}

Spawner* Spawner::Create()
{
    Spawner* pInstance = new Spawner();
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Spawner");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void Spawner::Free()
{
    __super::Free();

    m_MobDescs.clear();
    SafeRelease(m_pEngineUtility);
}