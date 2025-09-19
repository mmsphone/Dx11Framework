#include "Scene.h"
#include "EngineUtility.h"

Scene::Scene()
    :m_pEngineUtility { EngineUtility::GetInstance() }
{
    Safe_AddRef(m_pEngineUtility);
}

HRESULT Scene::Initialize()
{
    return S_OK;
}

void Scene::Update(_float fTimeDelta)
{
}

HRESULT Scene::Render()
{
    return S_OK;
}

void Scene::Free()
{
    __super::Free();

    m_pEngineUtility->DestroyInstance();
}
