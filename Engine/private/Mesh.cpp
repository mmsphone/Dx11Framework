#include "Mesh.h"

#include "EngineUtility.h"
#include "Bone.h"
#include "Shader.h"

Mesh::Mesh()
    : VertexIndexBuffer{ }
{
}

Mesh::Mesh(const Mesh& Prototype)
    : VertexIndexBuffer{ Prototype }
{
}

_uint Mesh::GetMaterialIndex() const
{
	return m_iMaterialIndex;
}

HRESULT Mesh::InitializePrototype(MODELTYPE eType, const MeshData& mesh, const vector<class Bone*>& Bones, _fmatrix PreTransformMatrix)
{
    strcpy_s(m_szName, mesh.name.c_str());
    m_iMaterialIndex = mesh.materialIndex;
    m_iNumVertexBuffers = 1;
    m_iNumVertices = (_uint)mesh.positions.size();

    m_iNumIndices = (_uint)mesh.indices.size();
    m_iIndexStride = 4;
    m_eIndexFormat = DXGI_FORMAT_R32_UINT;
    m_ePrimitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    HRESULT hr = (eType == MODELTYPE::NONANIM) ?
        ReadyVertexBufferForNonAnim(mesh, PreTransformMatrix) :
        ReadyVertexBufferForAnim(mesh, Bones);

    if (FAILED(hr)) return E_FAIL;

    // INDEX_BUFFER
    D3D11_BUFFER_DESC IBDesc{};
    IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA IndexInitialData{};
    IndexInitialData.pSysMem = mesh.indices.data();

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&IBDesc, &IndexInitialData, &m_pIndexBuffer)))
        return E_FAIL;

    return S_OK;
}

HRESULT Mesh::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT Mesh::BindBoneMatrices(Shader* pShader, const _char* pConstantName, const vector<Bone*>& Bones)
{
    for (size_t i = 0; i < m_iNumBones; i++)
    {
        XMStoreFloat4x4(&m_BoneMatrices[i], XMLoadFloat4x4(&m_OffsetMatrices[i]) * Bones[m_BoneIndices[i]]->GetCombinedTransformationMatrix());
    }

    return pShader->BindMatrices(pConstantName, m_BoneMatrices, m_iNumBones);
}

HRESULT Mesh::ReadyVertexBufferForNonAnim(const MeshData& mesh, _fmatrix PreTransformMatrix)
{
    m_iVertexStride = sizeof(VTXMESH);

    D3D11_BUFFER_DESC VBDesc{};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    std::vector<VTXMESH> vertices(m_iNumVertices);

    for (size_t i = 0; i < m_iNumVertices; i++)
    {
        vertices[i].vPosition = mesh.positions[i];
        XMStoreFloat3(&vertices[i].vPosition,
            XMVector3TransformCoord(XMLoadFloat3(&vertices[i].vPosition), PreTransformMatrix));

        vertices[i].vNormal = mesh.normals[i];
        XMStoreFloat3(&vertices[i].vNormal,
            XMVector3TransformNormal(XMLoadFloat3(&vertices[i].vNormal), PreTransformMatrix));

        if (i < mesh.texcoords.size())
            vertices[i].vTexcoord = mesh.texcoords[i];

        if (i < mesh.tangents.size())
            vertices[i].vTangent = mesh.tangents[i];
    }

    D3D11_SUBRESOURCE_DATA VertexInitialData{};
    VertexInitialData.pSysMem = vertices.data();

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&VBDesc, &VertexInitialData, &m_pVertexBuffer)))
        return E_FAIL;

    return S_OK;
}

HRESULT Mesh::ReadyVertexBufferForAnim(const MeshData& mesh, const vector<class Bone*>& Bones)
{
    m_iVertexStride = sizeof(VTXSKINMESH);

    D3D11_BUFFER_DESC VBDesc{};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    std::vector<VTXSKINMESH> vertices(m_iNumVertices);

    for (size_t i = 0; i < m_iNumVertices; i++)
    {
        vertices[i].vPosition = mesh.positions[i];
        vertices[i].vNormal = mesh.normals[i];

        if (i < mesh.texcoords.size())
            vertices[i].vTexcoord = mesh.texcoords[i];
        if (i < mesh.tangents.size())
            vertices[i].vTangent = mesh.tangents[i];
    }

    // 뼈 데이터 세팅
    m_iNumBones = (_uint)mesh.bones.size();
    m_OffsetMatrices.reserve(m_iNumBones);

    for (size_t i = 0; i < mesh.bones.size(); i++)
    {
        const MeshBone& bone = mesh.bones[i];

        _float4x4 OffsetMatrix = bone.offsetMatrix;
        XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));
        m_OffsetMatrices.push_back(OffsetMatrix);

        _uint boneIndex = 0;
        auto iter = std::find_if(Bones.begin(), Bones.end(),
            [&](Bone* pBone) {
                if (pBone->CompareName(bone.name.c_str()))
                    return true;
                ++boneIndex;
                return false;
            });

        if (iter == Bones.end())
            return E_FAIL;

        m_BoneIndices.push_back(boneIndex);

        // 가중치 적용
        for (auto& w : bone.weights)
        {
            if (w.vertexId >= vertices.size()) continue;

            auto& v = vertices[w.vertexId];
            if (v.vBlendWeight.x == 0.0f) { v.vBlendIndex.x = i; v.vBlendWeight.x = w.weight; }
            else if (v.vBlendWeight.y == 0.0f) { v.vBlendIndex.y = i; v.vBlendWeight.y = w.weight; }
            else if (v.vBlendWeight.z == 0.0f) { v.vBlendIndex.z = i; v.vBlendWeight.z = w.weight; }
            else if (v.vBlendWeight.w == 0.0f) { v.vBlendIndex.w = i; v.vBlendWeight.w = w.weight; }
        }
    }

    D3D11_SUBRESOURCE_DATA VertexInitialData{};
    VertexInitialData.pSysMem = vertices.data();

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&VBDesc, &VertexInitialData, &m_pVertexBuffer)))
        return E_FAIL;

    return S_OK;
}

Mesh* Mesh::Create(MODELTYPE eType, const MeshData& mesh, const vector<class Bone*>& Bones, _fmatrix PreTransformMatrix)
{
    Mesh* pInstance = new Mesh();

    if (FAILED(pInstance->InitializePrototype(eType, mesh, Bones, PreTransformMatrix)))
    {
        MSG_BOX("Failed to Created : Mesh");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Component* Mesh::Clone(void* pArg)
{
    Mesh* pInstance = new Mesh(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Mesh");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Mesh::Free()
{
    __super::Free();
}
