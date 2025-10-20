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
    if (m_iCurrentAnimIndex >= m_Animations.size())
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
    
    m_pModelData = pModelData;
    if (m_pModelData == nullptr) 
        return;

    ReadyBones(m_pModelData->rootNode, -1);
    ReadyMeshes();
    ReadyMaterials();
    ReadyAnimations();

    m_iCurrentAnimIndex = 0;
    m_isAnimLoop = true;
    m_isAnimFinished = false;
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

void Model::SetMeshVisible(_uint iMeshIndex, _bool bVisible)
{
    if (iMeshIndex > m_iNumMeshes) return;
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
    UpdateNonAnimVertexBuffer();
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

    for (auto& pAnimation : m_Animations)
        SafeRelease(pAnimation);
    m_Animations.clear();

    for (auto& pBone : m_Bones)
        SafeRelease(pBone);
    m_Bones.clear();

    for (auto& pMaterial : m_Materials)
        SafeRelease(pMaterial);
    m_Materials.clear();

    for (auto& pMesh : m_Meshes)
        SafeRelease(pMesh);
    m_Meshes.clear();

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
     
    if (m_pModelData)
    {
        SafeDelete(m_pModelData);
        m_pModelData = nullptr;
    }

    m_iCurrentAnimIndex = 0;
    m_isAnimLoop = false;
    m_isAnimFinished = false;
}

ModelData* Model::LoadNoAssimpModel(const _char* pFilePath)
{
    std::ifstream file(pFilePath, std::ios::binary);
    if (!file.is_open())
        return nullptr;

    ModelData* pModelData = new ModelData();

    // ---------------------
    // 1. Meshes
    _uint numMeshes = 0;
    file.read(reinterpret_cast<char*>(&numMeshes), sizeof(_uint));
    pModelData->meshes.resize(numMeshes);

    for (_uint i = 0; i < numMeshes; i++)
    {
        MeshData& mesh = pModelData->meshes[i];

        // 이름
        _uint nameLen = 0;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(_uint));
        mesh.name.resize(nameLen);
        file.read(mesh.name.data(), nameLen);

        file.read(reinterpret_cast<char*>(&mesh.materialIndex), sizeof(_uint));

        // positions
        _uint numPos = 0;
        file.read(reinterpret_cast<char*>(&numPos), sizeof(_uint));
        mesh.positions.resize(numPos);
        file.read(reinterpret_cast<char*>(mesh.positions.data()), sizeof(_float3) * numPos);

        // normals
        _uint numNormals = 0;
        file.read(reinterpret_cast<char*>(&numNormals), sizeof(_uint));
        mesh.normals.resize(numNormals);
        file.read(reinterpret_cast<char*>(mesh.normals.data()), sizeof(_float3) * numNormals);

        // texcoords
        _uint numTex = 0;
        file.read(reinterpret_cast<char*>(&numTex), sizeof(_uint));
        mesh.texcoords.resize(numTex);
        file.read(reinterpret_cast<char*>(mesh.texcoords.data()), sizeof(_float2) * numTex);

        // tangents
        _uint numTang = 0;
        file.read(reinterpret_cast<char*>(&numTang), sizeof(_uint));
        mesh.tangents.resize(numTang);
        file.read(reinterpret_cast<char*>(mesh.tangents.data()), sizeof(_float3) * numTang);

        // indices
        _uint numIndices = 0;
        file.read(reinterpret_cast<char*>(&numIndices), sizeof(_uint));
        mesh.indices.resize(numIndices);
        file.read(reinterpret_cast<char*>(mesh.indices.data()), sizeof(_uint) * numIndices);

        // bones
        _uint numBones = 0;
        file.read(reinterpret_cast<char*>(&numBones), sizeof(_uint));
        mesh.bones.resize(numBones);
        for (_uint b = 0; b < numBones; b++)
        {
            MeshBone& bone = mesh.bones[b];

            // bone name
            _uint len = 0;
            file.read(reinterpret_cast<char*>(&len), sizeof(_uint));
            bone.name.resize(len);
            file.read(bone.name.data(), len);

            // weights
            _uint numWeights = 0;
            file.read(reinterpret_cast<char*>(&numWeights), sizeof(_uint));
            bone.weights.resize(numWeights);
            file.read(reinterpret_cast<char*>(bone.weights.data()), sizeof(VertexWeight) * numWeights);

            // offset matrix
            file.read(reinterpret_cast<char*>(&bone.offsetMatrix), sizeof(_float4x4));
        }
    }

    // ---------------------
    // 2. Materials
    _uint numMaterials = 0;
    file.read(reinterpret_cast<char*>(&numMaterials), sizeof(_uint));
    pModelData->materials.resize(numMaterials);

    for (_uint i = 0; i < numMaterials; i++)
    {
        MaterialData& mat = pModelData->materials[i];

        // name
        _uint nameLen = 0;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(_uint));
        mat.name.resize(nameLen);
        file.read(mat.name.data(), nameLen);

        // textures
        for (int t = 0; t < (int)TextureType::End; t++)
        {
            _uint numTex = 0;
            file.read(reinterpret_cast<char*>(&numTex), sizeof(_uint));
            mat.texturePaths[t].resize(numTex);
            for (_uint j = 0; j < numTex; j++)
            {
                _uint pathLen = 0;
                file.read(reinterpret_cast<char*>(&pathLen), sizeof(_uint));
                mat.texturePaths[t][j].resize(pathLen);
                file.read(mat.texturePaths[t][j].data(), pathLen);
            }
        }
    }

    // ---------------------
    // 3. Node hierarchy
    std::function<void(NodeData&)> readNode;
    readNode = [&](NodeData& node)
    {
        _uint len = 0;
        file.read(reinterpret_cast<char*>(&len), sizeof(_uint));
        node.name.resize(len);
        file.read(node.name.data(), len);

        file.read(reinterpret_cast<char*>(&node.parentIndex), sizeof(_int));
        file.read(reinterpret_cast<char*>(&node.transform), sizeof(_float4x4));

        _uint numChildren = 0;
        file.read(reinterpret_cast<char*>(&numChildren), sizeof(_uint));
        node.children.resize(numChildren);
        for (_uint c = 0; c < numChildren; c++)
        {
            NodeData child;
            readNode(child);
            node.children[c] = child;
        }
    };

    readNode(pModelData->rootNode);

    // ---------------------
    // 4. Animations
    _uint numAnims = 0;
    file.read(reinterpret_cast<char*>(&numAnims), sizeof(_uint));
    pModelData->animations.resize(numAnims);

    for (_uint i = 0; i < numAnims; i++)
    {
        AnimationData& anim = pModelData->animations[i];

        _uint nameLen = 0;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(_uint));
        anim.name.resize(nameLen);
        file.read(anim.name.data(), nameLen);

        file.read(reinterpret_cast<char*>(&anim.duration), sizeof(float));
        file.read(reinterpret_cast<char*>(&anim.ticksPerSecond), sizeof(float));

        // channels
        _uint numChannels = 0;
        file.read(reinterpret_cast<char*>(&numChannels), sizeof(_uint));
        anim.channels.resize(numChannels);

        for (_uint c = 0; c < numChannels; c++)
        {
            ChannelData& ch = anim.channels[c];

            _uint nodeNameLen = 0;
            file.read(reinterpret_cast<char*>(&nodeNameLen), sizeof(_uint));
            ch.nodeName.resize(nodeNameLen);
            file.read(ch.nodeName.data(), nodeNameLen);

            // position keys
            _uint numPosKeys = 0;
            file.read(reinterpret_cast<char*>(&numPosKeys), sizeof(_uint));
            ch.positionKeys.resize(numPosKeys);
            file.read(reinterpret_cast<char*>(ch.positionKeys.data()), sizeof(KeyVector) * numPosKeys);

            // rotation keys
            _uint numRotKeys = 0;
            file.read(reinterpret_cast<char*>(&numRotKeys), sizeof(_uint));
            ch.rotationKeys.resize(numRotKeys);
            file.read(reinterpret_cast<char*>(ch.rotationKeys.data()), sizeof(KeyQuat) * numRotKeys);

            // scaling keys
            _uint numScaleKeys = 0;
            file.read(reinterpret_cast<char*>(&numScaleKeys), sizeof(_uint));
            ch.scalingKeys.resize(numScaleKeys);
            file.read(reinterpret_cast<char*>(ch.scalingKeys.data()), sizeof(KeyVector) * numScaleKeys);
        }
    }

    file.close();
    return pModelData;
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
