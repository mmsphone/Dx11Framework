#include "TestTerrain.h"

#include "EngineUtility.h"

#include "Cell.h"

TestTerrain::TestTerrain()
    : Terrain{ }
{
}

TestTerrain::TestTerrain(const TestTerrain& Prototype)
    : Terrain{ Prototype }
{
}

HRESULT TestTerrain::InitializePrototype()
{
    return S_OK;
}

HRESULT TestTerrain::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    m_vBrushPos = _float4(20.f, 0.f, 20.f, 1.f);
    m_fBrushRange = _float(3.f);
    m_fIncTexSize = _float(30.f);
    return S_OK;
}

void TestTerrain::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void TestTerrain::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void TestTerrain::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);
    __super::LateUpdate(fTimeDelta);
}

HRESULT TestTerrain::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Texture* pTextureDiffuse = dynamic_cast<Texture*>(FindComponent(TEXT("Texture_Diffuse")));
    Texture* pTextureMask = dynamic_cast<Texture*>(FindComponent(TEXT("Texture_Mask")));
    Texture* pTextureBrush = dynamic_cast<Texture*>(FindComponent(TEXT("Texture_Brush")));
    VIBufferTerrain* pTerrain = dynamic_cast<VIBufferTerrain*>(FindComponent(TEXT("VIBuffer")));

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION))))
        return E_FAIL;

    if (FAILED(pTextureDiffuse->BindShaderResources(pShader, "g_DiffuseTexture")))
        return E_FAIL;
    if (FAILED(pTextureMask->BindRenderTargetShaderResource(pShader, "g_MaskTexture", 0)))
        return E_FAIL;
    if (FAILED(pTextureBrush->BindRenderTargetShaderResource(pShader, "g_BrushTexture", 0)))
        return E_FAIL;

    if (FAILED(pShader->BindRawValue("g_vCamPosition", m_pEngineUtility->GetCamPosition(), sizeof(_float4))))
        return E_FAIL;

    if (FAILED(pShader->BindRawValue("g_vBrushPos", &m_vBrushPos, sizeof(_float4))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_fBrushRange", &m_fBrushRange, sizeof(_float))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_fIncTexSize", &m_fIncTexSize, sizeof(_float))))
        return E_FAIL;

    pShader->Begin(0);
    pTerrain->BindBuffers();
    pTerrain->Render();

    return S_OK;
}

void TestTerrain::SetBrushPos(const _float4& vPos)
{
    m_vBrushPos = vPos;
}

_float4 TestTerrain::GetBrushPos() const
{
    return m_vBrushPos;
}

_float TestTerrain::GetIncTexSize() const
{
    return m_fIncTexSize;
}

HRESULT TestTerrain::ReadyComponents()
{
    /* For.Com_VIBuffer */
    if (FAILED(AddComponent(SCENE::MAP, TEXT("VIBufferTerrain"), TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;
    /* For.Com_Texture_Diffuse */
    if (FAILED(AddComponent(SCENE::MAP, TEXT("Texture_Terrain_Diffuse"), TEXT("Texture_Diffuse"), nullptr, nullptr)))
        return E_FAIL;
    /* For.Com_Texture_Mask */
    if (FAILED(AddComponent(SCENE::MAP, TEXT("Texture_Terrain_Mask"), TEXT("Texture_Mask"), nullptr, nullptr)))
        return E_FAIL;
    /* For.Com_Texture_Brush */
    if (FAILED(AddComponent(SCENE::MAP, TEXT("Texture_Terrain_Brush"), TEXT("Texture_Brush"), nullptr, nullptr)))
        return E_FAIL;
    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::MAP, TEXT("Shader_VtxTerrain"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

TestTerrain* TestTerrain::Create()
{
    TestTerrain* pInstance = new TestTerrain();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : TestTerrain");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* TestTerrain::Clone(void* pArg)
{
    TestTerrain* pInstance = new TestTerrain(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : TestTerrain");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void TestTerrain::Free()
{
    __super::Free();
}
