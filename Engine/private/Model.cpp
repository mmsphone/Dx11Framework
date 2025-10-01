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

    // 여기서는 Assimp을 쓰지 않고, 외부에서 준비된 NAS_Model 데이터를 받는다고 가정
    // (예: 로더가 json/이진파일 → NAS_Model로 변환한 뒤 전달)
    m_pModelData = LoadNoAssimpModel(pModelFilePath);
    if (m_pModelData == nullptr)
        return E_FAIL;

    XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

    if (FAILED(ReadyBones(m_pModelData->rootNode, -1)))
        return E_FAIL;

    if (FAILED(ReadyMeshes()))
        return E_FAIL;

    if (FAILED(ReadyMaterials(pModelFilePath)))
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
    m_Animations[m_iCurrentAnimIndex]->UpdateTransformationMatrix(fTimeDelta, m_Bones, m_isAnimLoop, &m_isAnimFinished);

    for (auto& pBone : m_Bones)
    {
        pBone->UpdateCombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
}

void Model::SetAnimation(_uint iIndex, _bool isLoop)
{
    if (m_iCurrentAnimIndex == iIndex && m_isAnimLoop == isLoop)
        return;

    m_iCurrentAnimIndex = iIndex;
    m_isAnimLoop = isLoop;

    m_Animations[iIndex]->Reset();
}

_bool Model::isAnimFinished() const
{
    return m_isAnimFinished;
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

HRESULT Model::ReadyMaterials(const _char* pModelFilePath)
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

ModelData* Model::LoadNoAssimpModel(const _char* pFilePath)
{
    std::ifstream file(pFilePath, std::ios::binary);
    if (!file.is_open())
        return nullptr;

    ModelData* pModelData = new ModelData();

    // ---------------------
    // 1. meshes
    _uint numMeshes = 0;
    file.read(reinterpret_cast<char*>(&numMeshes), sizeof(_uint));
    pModelData->meshes.resize(numMeshes);

    for (_uint i = 0; i < numMeshes; i++)
    {
        MeshData& mesh = pModelData->meshes[i];

        // 이름 길이 + 문자열
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

            // name
            _uint len = 0;
            file.read(reinterpret_cast<char*>(&len), sizeof(_uint));
            bone.name.resize(len);
            file.read(bone.name.data(), len);

            // weights
            _uint numWeights = 0;
            file.read(reinterpret_cast<char*>(&numWeights), sizeof(_uint));
            bone.weights.resize(numWeights);
            file.read(reinterpret_cast<char*>(bone.weights.data()), sizeof(VertexWeight) * numWeights);

            // offsetMatrix
            file.read(reinterpret_cast<char*>(&bone.offsetMatrix), sizeof(_float4x4));
        }
    }

    // ---------------------
    // 2. materials
    _uint numMaterials = 0;
    file.read(reinterpret_cast<char*>(&numMaterials), sizeof(_uint));
    pModelData->materials.resize(numMaterials);
    for (_uint i = 0; i < numMaterials; i++)
    {
        MaterialData& mat = pModelData->materials[i];

        // name
        _uint len = 0;
        file.read(reinterpret_cast<char*>(&len), sizeof(_uint));
        mat.name.resize(len);
        file.read(mat.name.data(), len);

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
    // 3. root node
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
    // 4. animations
    _uint numAnims = 0;
    file.read(reinterpret_cast<char*>(&numAnims), sizeof(_uint));
    pModelData->animations.resize(numAnims);
    for (_uint i = 0; i < numAnims; i++)
    {
        AnimationData& anim = pModelData->animations[i];

        _uint len = 0;
        file.read(reinterpret_cast<char*>(&len), sizeof(_uint));
        anim.name.resize(len);
        file.read(anim.name.data(), len);

        file.read(reinterpret_cast<char*>(&anim.duration), sizeof(float));
        file.read(reinterpret_cast<char*>(&anim.ticksPerSecond), sizeof(float));
    }

    file.close();
    return pModelData;
}
