#include "Model.h"

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
    , m_PreTransformMatrix{ Prototype.m_PreTransformMatrix }
    , m_iNumAnimations{ Prototype.m_iNumAnimations }
    , m_pModelData{ Prototype.m_pModelData }
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

    m_pModelData = LoadNoAssimpModel(pModelFilePath);
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

HRESULT Model::BindShaderResource(_uint iMeshIndex, Shader* pShader, const _char* pConstantName, TextureType eType, _uint iIndex)
{
    _uint iMaterialIndex = m_Meshes[iMeshIndex]->GetMaterialIndex();
    if (iMaterialIndex >= m_iNumMaterials)
        return E_FAIL;

    return m_Materials[iMaterialIndex]->BindShaderResource(pShader, pConstantName, eType, iIndex);
}

HRESULT Model::BindBoneMatrices(_uint iMeshIndex, Shader* pShader, const _char* pConstantName)
{
    if (iMeshIndex >= m_iNumMeshes)
        return E_FAIL;

    return m_Meshes[iMeshIndex]->BindBoneMatrices(pShader, pConstantName, m_Bones);
}

void Model::PlayAnimation(_float fTimeDelta)
{
    if (m_isAnimStop == true || m_iCurrentAnimIndex >= m_Animations.size())
        return;
    m_Animations[m_iCurrentAnimIndex]->UpdateTransformationMatrix(fTimeDelta, m_Bones, m_isAnimLoop, &m_isAnimFinished);

    for (auto& pBone : m_Bones)
    {
        pBone->UpdateCombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
}

void Model::SetAnimation(_uint iIndex, _bool isLoop)
{
    if (iIndex >= m_Animations.size())
        return;
    
    if (m_iCurrentAnimIndex == iIndex && m_isAnimLoop == isLoop)
        return;

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

_uint Model::GetCurrentAnimIndex()
{
    return m_iCurrentAnimIndex;
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
    m_isAnimLoop = false;
    m_isAnimFinished = false;
    m_isAnimStop = true;
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
    if(m_isCloned == false)
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

ModelData* Model::LoadNoAssimpModel(const _char* path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())return nullptr;
    auto m = new ModelData();

    auto R = [&](auto& v, size_t s) {_uint n; f.read((char*)&n, 4); v.resize(n); if (n)f.read((char*)v.data(), s * n); };

    // 1. Meshes
    _uint nm; f.read((char*)&nm, 4); m->meshes.resize(nm);
    for (auto& mesh : m->meshes) {
        _uint l; f.read((char*)&l, 4); mesh.name.resize(l); f.read(mesh.name.data(), l);
        f.read((char*)&mesh.materialIndex, 4);
        R(mesh.positions, sizeof(_float3)); R(mesh.normals, sizeof(_float3));
        R(mesh.texcoords, sizeof(_float2)); R(mesh.tangents, sizeof(_float3)); R(mesh.indices, sizeof(_uint));

        _uint nb; f.read((char*)&nb, 4); mesh.bones.resize(nb);
        for (auto& b : mesh.bones) {
            _uint ln; f.read((char*)&ln, 4); b.name.resize(ln); f.read(b.name.data(), ln);
            _uint nw; f.read((char*)&nw, 4); b.weights.resize(nw);
            if (nw)f.read((char*)b.weights.data(), sizeof(VertexWeight) * nw);
            f.read((char*)&b.offsetMatrix, sizeof(_float4x4));
        }
    }

    // 2. Materials
    _uint nmat; f.read((char*)&nmat, 4); m->materials.resize(nmat);
    for (auto& mt : m->materials) {
        _uint l; f.read((char*)&l, 4); mt.name.resize(l); f.read(mt.name.data(), l);
        for (int t = 0; t < (int)TextureType::End; t++) {
            _uint nt; f.read((char*)&nt, 4); mt.texturePaths[t].resize(nt);
            for (auto& p : mt.texturePaths[t]) {
                _uint pl; f.read((char*)&pl, 4); p.resize(pl); f.read(p.data(), pl);
            }
        }
    }

    // 3. Node
    std::function<void(NodeData&)> RN = [&](NodeData& n) {
        _uint l; f.read((char*)&l, 4); n.name.resize(l); f.read(n.name.data(), l);
        f.read((char*)&n.parentIndex, 4);
        f.read((char*)&n.transform, sizeof(_float4x4));
        _uint c; f.read((char*)&c, 4); n.children.resize(c);
        for (auto& ch : n.children)RN(ch);
    };
    RN(m->rootNode);

    // 4. Animations
    _uint na; f.read((char*)&na, 4); m->animations.resize(na);
    for (auto& a : m->animations) {
        _uint l; f.read((char*)&l, 4); a.name.resize(l); f.read(a.name.data(), l);
        f.read((char*)&a.duration, 4); f.read((char*)&a.ticksPerSecond, 4);
        _uint nc; f.read((char*)&nc, 4); a.channels.resize(nc);
        for (auto& ch : a.channels) {
            _uint ln; f.read((char*)&ln, 4); ch.nodeName.resize(ln); f.read(ch.nodeName.data(), ln);
            auto RK = [&](auto& v, size_t s) {_uint n; f.read((char*)&n, 4); v.resize(n); if (n)f.read((char*)v.data(), s * n); };
            RK(ch.positionKeys, sizeof(KeyVector));
            RK(ch.rotationKeys, sizeof(KeyQuat));
            RK(ch.scalingKeys, sizeof(KeyVector));
        }
    }

    // 5. Global Bones
    _uint nb; f.read((char*)&nb, 4); m->bones.resize(nb);
    for (auto& b : m->bones) {
        _uint l; f.read((char*)&l, 4); b.name.resize(l); f.read(b.name.data(), l);
        f.read((char*)&b.offsetMatrix, sizeof(_float4x4));
    }

    f.close(); return m;
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
