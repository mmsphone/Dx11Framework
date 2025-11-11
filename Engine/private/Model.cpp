#include "Model.h"

#include "EngineUtility.h"

#include "Mesh.h"
#include "Bone.h"
#include "Material.h"
#include "Animation.h"

Model::Model()
	: Component{ }
{
}

Model::Model(const Model& Prototype)
	: Component{ Prototype}
    , m_iNumMeshes{ Prototype.m_iNumMeshes }
    , m_Meshes{ Prototype.m_Meshes }
    , m_iNumMaterials{ Prototype.m_iNumMaterials }
    , m_Materials{ Prototype.m_Materials }
    , m_pModelData{ Prototype.m_pModelData }
    , m_PreTransformMatrix{ Prototype.m_PreTransformMatrix }
    , m_iNumAnimations{ Prototype.m_iNumAnimations }
{
    for (auto& pPrototypeAnim : Prototype.m_Animations)
        m_Animations.push_back(pPrototypeAnim->Clone());

    for (auto& pPrototypeBone : Prototype.m_Bones)
        m_Bones.push_back(pPrototypeBone->Clone());

    for (auto& pMesh : m_Meshes)
        SafeAddRef(pMesh);
    
    for (auto& pMaterial : m_Materials)
        SafeAddRef(pMaterial);
}

_uint Model::GetNumMeshes() const
{
    return m_iNumMeshes;
}

const _float4x4* Model::GetSocketBoneMatrixPtr(const _char* pBoneName) const
{
    auto iter = find_if(m_Bones.begin(), m_Bones.end(), [&](Bone* pBone)->_bool {
        return pBone->CompareName(pBoneName);
        });

    if (iter == m_Bones.end())
        return nullptr;

    return (*iter)->GetCombinedTransformationMatrixPtr();
}

HRESULT Model::InitializePrototype(MODELTYPE eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
    m_eType = eType;

    m_pModelFilePath = pModelFilePath;
    m_pModelData = m_pEngineUtility->LoadNoAssimpModel(pModelFilePath);
    if (m_pModelData == nullptr)
        return E_FAIL;

    XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

    if (FAILED(ReadyBones(m_pModelData->rootNode, -1)))
        return E_FAIL;

    if (FAILED(ReadyMeshes()))
        return E_FAIL;

    if (FAILED(ReadyMaterials()))
        return E_FAIL;

    if (FAILED(ReadyAnimations()))
        return E_FAIL;
    UpdateBoneMatrices();

    return S_OK;
}

HRESULT Model::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT Model::Render(_uint iMeshIndex)
{
    if (iMeshIndex > m_iNumMeshes || m_Meshes.at(iMeshIndex)->IsVisible() == false) return S_OK;

    m_Meshes[iMeshIndex]->BindBuffers();
    m_Meshes[iMeshIndex]->Render();

    return S_OK;
}

HRESULT Model::BindRenderTargetShaderResource(_uint iMeshIndex, Shader* pShader, const _char* pConstantName, TextureType eType, _uint iIndex)
{
    _uint iMaterialIndex = m_Meshes[iMeshIndex]->GetMaterialIndex();
    if (iMaterialIndex >= m_iNumMaterials)
        return E_FAIL;

    return m_Materials[iMaterialIndex]->BindRenderTargetShaderResource(pShader, pConstantName, eType, iIndex);
}

HRESULT Model::BindBoneMatrices(_uint iMeshIndex, Shader* pShader, const _char* pConstantName)
{
    if (iMeshIndex >= m_iNumMeshes)
        return E_FAIL;

    return m_Meshes[iMeshIndex]->BindBoneMatrices(pShader, pConstantName, m_Bones);
}

void Model::UpdateBoneMatrices()
{
    for (auto& pBone : m_Bones)
    {
        pBone->UpdateCombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
}

void Model::PlayAnimation(_float fTimeDelta)
{
    if (m_isAnimStop == true || m_iCurrentAnimIndex >= m_Animations.size())
        return;
    m_Animations[m_iCurrentAnimIndex]->UpdateTransformationMatrix(fTimeDelta, m_Bones, m_isAnimLoop, &m_isAnimFinished, m_pModelData);

    if (m_IsBlending)
    {
        m_BlendElapsed += fTimeDelta;
        float alpha = (m_BlendDuration > 0.f) ? (m_BlendElapsed / m_BlendDuration) : 1.f;
        alpha = std::clamp(alpha, 0.f, 1.f);

        // ■■■ 강제 가시화 디버그: 처음 0.2초 동안 alpha를 0.5로 고정해보자
        // (보간이 ‘전혀’ 안 보이면 이걸로 경로가 타는지부터 확인)
        // if (m_BlendElapsed < 0.2f) alpha = 0.5f;

        const size_t N = m_Bones.size();
        const size_t M = min(N, m_BlendFromLocal.size());
        for (size_t i = 0; i < M; ++i)
        {
            const _float4x4& fromLocal = m_BlendFromLocal[i];

            _float4x4 toLocal{};
            if (auto pTo = m_Bones[i]->GetLocalTransformationMatrixPtr())
                toLocal = *pTo;
            else
                XMStoreFloat4x4(&toLocal, XMMatrixIdentity());

            _float4x4 blendedLocal = LerpSRT_Local(fromLocal, toLocal, alpha);
            m_Bones[i]->SetTransformationMatrix(XMLoadFloat4x4(&blendedLocal));
        }

        if (alpha >= 1.f) {
            m_IsBlending = false;
            m_BlendFromLocal.clear();
            m_BlendFromLocal.shrink_to_fit();
        }
    }

    UpdateBoneMatrices();

}

void Model::SetAnimation(_uint iIndex, _bool isLoop)
{
    SetAnimation(iIndex, isLoop, 0.1f);
}

void Model::SetAnimation(_uint iIndex, _bool isLoop, _float blendTimeSec, _bool restartFlag)
{
    if (iIndex >= m_Animations.size())
        return;

    if (m_iCurrentAnimIndex == iIndex && m_isAnimLoop == isLoop && restartFlag == false)
        return;
    
    if (restartFlag == true)
        blendTimeSec = 0.f;

    // 1) 이전 로컬 포즈 캡처
    m_BlendFromLocal.clear();
    m_BlendFromLocal.reserve(m_Bones.size());
    for (auto* pBone : m_Bones)
    {
        _float4x4 cur{};
        if (auto p = pBone->GetLocalTransformationMatrixPtr())
            cur = *p;
        else
            XMStoreFloat4x4(&cur, XMMatrixIdentity());
        m_BlendFromLocal.push_back(cur);
    }

    // 2) 블렌딩 타이머 초기화
    m_IsBlending = (blendTimeSec > 0.f);
    m_BlendElapsed = 0.f;
    m_BlendDuration = max(0.f, blendTimeSec);

    // 3) 대상 애니로 전환
    m_isAnimFinished = false;
    m_iCurrentAnimIndex = iIndex;
    m_isAnimLoop = isLoop;
    m_Animations[iIndex]->Reset();
}

void Model::StopAnimation()
{
    m_isAnimStop = true;
}

void Model::ResumeAnimation()
{
    m_isAnimStop = false;
}

_uint Model::GetCurrentAnimIndex() const
{
    return m_iCurrentAnimIndex;
}

_float Model::GetCurAnimTrackPos() const
{
    if(m_Animations.size() > 0)
        return  m_Animations[m_iCurrentAnimIndex]->GetCurTrackPos();
    return 0.f;
}

_float Model::GetCurAnimDuration() const
{
    if (m_Animations.size() > 0)
        return  m_Animations[m_iCurrentAnimIndex]->GetDuration();
    return 0.f;
}

_bool Model::isAnimFinished() const
{
    return m_isAnimFinished;
}

ModelData* Model::GetModelData() const
{
    return m_pModelData;
}

void Model::SetModelData(ModelData* pModelData)
{
    ClearModelData();

    if (m_pModelData)
    {
        SafeDelete(m_pModelData);
    }

    m_pModelData = pModelData;
    if (m_pModelData == nullptr) 
        return;

    ReadyBones(m_pModelData->rootNode, -1);
    ReadyMeshes();
    ReadyMaterials();
    ReadyAnimations();

    m_iCurrentAnimIndex = 0;
}

void Model::SetTargetMesh(_int iMeshIndex)
{
    for (_uint i = 0; i < m_iNumMeshes; i++) {
        if (iMeshIndex == -1 || i == iMeshIndex)
            m_Meshes[i]->SetVisible(true);
        else
            m_Meshes[i]->SetVisible(false);
    }
}

const string& Model::GetBinPath() const
{
    static string emptyPath = "";
    if (m_pModelData)
        return m_pModelData->modelDataFilePath;
    return emptyPath;
}

void Model::SetBinPath(const string& binPath)
{
    m_pModelData->modelDataFilePath = binPath;
}

_bool Model::IsMeshVisible(_uint iMeshIndex)
{
    if (iMeshIndex > m_iNumMeshes) 
        return false;
    return m_Meshes.at(iMeshIndex)->IsVisible();
}

void Model::SetMeshVisible(_uint iMeshIndex, _bool bVisible)
{
    if (iMeshIndex > m_iNumMeshes) 
        return;
    m_Meshes.at(iMeshIndex)->SetVisible(bVisible);
}

void Model::SetPreTransformRotation(const _float3& _eulerDeg)
{
    _matrix curMat = XMLoadFloat4x4(&m_PreTransformMatrix);
    XMVECTOR Scale, RotationQuat, Translation;
    XMMatrixDecompose(&Scale, &RotationQuat, &Translation, curMat);

    const _float rotX = XMConvertToRadians(_eulerDeg.x); // X
    const _float rotY = XMConvertToRadians(_eulerDeg.y); // Y
    const _float rotZ = XMConvertToRadians(_eulerDeg.z); // Z

    _matrix ScaleMat = XMMatrixScalingFromVector(Scale);
    _matrix RotationMat = XMMatrixRotationRollPitchYaw(rotX, rotY, rotZ);
    _matrix TranslationMat = XMMatrixTranslationFromVector(Translation);

    _matrix newMat = ScaleMat * RotationMat * TranslationMat;
    XMStoreFloat4x4(&m_PreTransformMatrix, newMat);

    for (auto& pBone : m_Bones)
        pBone->UpdateCombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
}

_float3 Model::GetPreTransformRotation() const
{
    _matrix mat = XMLoadFloat4x4(&m_PreTransformMatrix);

    _vector Scale, RotationQuat, Translation;
    XMMatrixDecompose(&Scale, &RotationQuat, &Translation, mat);
    RotationQuat = XMQuaternionNormalize(RotationQuat);

    _float4 Q{};
    XMStoreFloat4(&Q, RotationQuat);

    float sinp = 2.0f * (Q.w * Q.x + Q.y * Q.z);
    float cosp = 1.0f - 2.0f * (Q.x * Q.x + Q.y * Q.y);
    float rotX = std::atan2(sinp, cosp);

    float siny = 2.0f * (Q.w * Q.y - Q.z * Q.x);
    siny = std::clamp(siny, -1.0f, 1.0f);
    float rotY = std::asin(siny);

    float sinr = 2.0f * (Q.w * Q.z + Q.x * Q.y);
    float cosr = 1.0f - 2.0f * (Q.y * Q.y + Q.z * Q.z);
    float rotZ = std::atan2(sinr, cosr);

    _float3 Deg{};
    Deg.x = XMConvertToDegrees(rotX);
    Deg.y = XMConvertToDegrees(rotY);
    Deg.z = XMConvertToDegrees(rotZ);

    return Deg;
}

Model* Model::Create(MODELTYPE eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
    Model* pInstance = new Model();

    if (FAILED(pInstance->InitializePrototype(eType, pModelFilePath, PreTransformMatrix)))
    {
        MSG_BOX("Failed to Created : Model");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Model* Model::Create(MODELTYPE eType, ModelData* pModelData, _fmatrix PreTransformMatrix)
{
    Model* pInstance = new Model();

    if (FAILED(pInstance->InitializePrototype(eType, pModelData, PreTransformMatrix)))
    {
        MSG_BOX("Failed to Created : Model (ModelData)");
        SafeRelease(pInstance);
    }

    return pInstance;

}

Component* Model::Clone(void* pArg)
{
    Model* pInstance = new Model(*this);

    if (m_pModelData)
        pInstance->m_pModelData = new ModelData(*m_pModelData);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Model");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Model::Free()
{
    __super::Free();

    ClearModelData();
    SafeDelete(m_pModelData);
}

HRESULT Model::ReadyMeshes()
{
    m_iNumMeshes = (_uint)m_pModelData->meshes.size();

    for (size_t i = 0; i < m_iNumMeshes; i++)
    {
        Mesh* pMesh = Mesh::Create(
            m_eType,
            m_pModelData->meshes[i],
            m_Bones,
            XMLoadFloat4x4(&m_PreTransformMatrix)
        );

        if (nullptr == pMesh)
            return E_FAIL;
        m_Meshes.push_back(pMesh);
    }

    return S_OK;
}

HRESULT Model::ReadyMaterials()
{
    m_iNumMaterials = (_uint)m_pModelData->materials.size();

    for (size_t i = 0; i < m_iNumMaterials; i++)
    {
        Material* pMaterial = Material::Create(
            m_pModelData->materials[i]
        );

        if (nullptr == pMaterial)
            return E_FAIL;

        m_Materials.push_back(pMaterial);
    }

    return S_OK;
}

HRESULT Model::ReadyBones(const NodeData& nodeData, _int iParentIndex)
{
    Bone* pBone = Bone::Create(&nodeData, iParentIndex);
    if (nullptr == pBone)
        return E_FAIL;

    m_Bones.push_back(pBone);

    _int iPIndex = (_int)m_Bones.size() - 1;

    for (auto& childNode : nodeData.children)
    {
        ReadyBones(childNode, iPIndex);
    }

    return S_OK;
}

HRESULT Model::ReadyAnimations()
{
    m_iNumAnimations = (_uint)m_pModelData->animations.size();

    for (size_t i = 0; i < m_iNumAnimations; i++)
    {
        Animation* pAnimation = Animation::Create(m_pModelData->animations[i], m_Bones);
        if (nullptr == pAnimation)
            return E_FAIL;

        m_Animations.push_back(pAnimation);
    }

    return S_OK;
}

void Model::ClearModelData()
{
    for (auto& pMesh : m_Meshes)
        SafeRelease(pMesh);
    m_Meshes.clear();
    m_iNumMeshes = 0;

    for (auto& pMaterial : m_Materials)
        SafeRelease(pMaterial);
    m_Materials.clear();
    m_iNumMaterials = 0;

    for (auto& pBone : m_Bones)
        SafeRelease(pBone);
    m_Bones.clear();

    for (auto& pAnimation : m_Animations)
        SafeRelease(pAnimation);
    m_Animations.clear();
    m_iNumAnimations = 0;
}

HRESULT Model::InitializePrototype(MODELTYPE eType, ModelData* pModelData, _fmatrix PreTransformMatrix)
{
    m_eType = eType;

    m_pModelData = pModelData;

    if (m_pModelData == nullptr)
        return E_FAIL;

    XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

    if (FAILED(ReadyBones(m_pModelData->rootNode, -1)))
        return E_FAIL;

    if (FAILED(ReadyMeshes()))
        return E_FAIL;

    if (FAILED(ReadyMaterials()))
        return E_FAIL;

    if (FAILED(ReadyAnimations()))
        return E_FAIL;

    return S_OK;
}

void Model::DecomposeSRT(const _float4x4& M, XMVECTOR& S, XMVECTOR& R, XMVECTOR& T)
{
    _matrix mm = XMLoadFloat4x4(&M);
    XMMatrixDecompose(&S, &R, &T, mm);
    R = XMQuaternionNormalize(R);
}

_float4x4 Model::ComposeSRT(const XMVECTOR& S, const XMVECTOR& R, const XMVECTOR& T)
{
    _matrix M = XMMatrixScalingFromVector(S)
        * XMMatrixRotationQuaternion(R)
        * XMMatrixTranslationFromVector(T);
    _float4x4 out{};
    XMStoreFloat4x4(&out, M);
    return out;
}

_float4x4 Model::LerpSRT_Local(const _float4x4& fromM, const _float4x4& toM, float alpha)
{
    alpha = std::clamp(alpha, 0.f, 1.f);

    XMVECTOR Sf, Rf, Tf;
    XMVECTOR St, Rt, Tt;
    DecomposeSRT(fromM, Sf, Rf, Tf);
    DecomposeSRT(toM, St, Rt, Tt);

    // 위치/스케일은 선형, 회전은 쿼터니언 SLERP
    XMVECTOR S = XMVectorLerp(Sf, St, alpha);
    XMVECTOR T = XMVectorLerp(Tf, Tt, alpha);
    XMVECTOR R = XMQuaternionSlerp(Rf, Rt, alpha);

    return ComposeSRT(S, R, T);
}
