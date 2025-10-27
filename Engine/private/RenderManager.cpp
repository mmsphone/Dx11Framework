#include "RenderManager.h"
#include "EngineUtility.h"
#include "Object.h"

RenderManager::RenderManager()
    :m_pEngineUtility{ EngineUtility::GetInstance() }
{
    SafeAddRef(m_pEngineUtility);
}

HRESULT RenderManager::Initialize()
{
    return S_OK;
}

HRESULT RenderManager::JoinRenderGroup(RENDERGROUP eGroupId, Object* pObject)
{
    m_RenderObjects[eGroupId].push_back(pObject);
    SafeAddRef(pObject);

    return S_OK;
}

void RenderManager::Draw()
{
    RenderPriority();
    RenderNonBlend();
    RenderBlend();
    RenderUI();
}

RenderManager* RenderManager::Create()
{
    RenderManager* pInstance = new RenderManager();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : RenderManager");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void RenderManager::Free()
{
    __super::Free();
    
    SafeRelease(m_pEngineUtility);
}

void RenderManager::RenderPriority()
{
    for (auto& pObject : m_RenderObjects[RENDERGROUP::PRIORITY])
    {
        if(pObject!= nullptr && pObject->IsDead() == false)
            pObject->Render();
        SafeRelease(pObject);
    }
    m_RenderObjects[RENDERGROUP::PRIORITY].clear();
}

void RenderManager::RenderNonBlend()
{
    for (auto& pObject : m_RenderObjects[RENDERGROUP::NONBLEND])
    {
        if (pObject != nullptr && pObject->IsDead() == false)
            pObject->Render();
        SafeRelease(pObject);
    }
    m_RenderObjects[RENDERGROUP::NONBLEND].clear();
}

void RenderManager::RenderBlend()
{
    for (auto& pObject : m_RenderObjects[RENDERGROUP::BLEND])
    {
        if (pObject != nullptr && pObject->IsDead() == false)
            pObject->Render();
        SafeRelease(pObject);
    }
    m_RenderObjects[RENDERGROUP::BLEND].clear();
}

void RenderManager::RenderUI()
{
    for (auto& pObject : m_RenderObjects[RENDERGROUP::UI])
    {
        if (pObject != nullptr && pObject->IsDead() == false)
            pObject->Render();
        SafeRelease(pObject);
    }
    m_RenderObjects[RENDERGROUP::UI].clear();
}
