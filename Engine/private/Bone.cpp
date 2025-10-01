#include "Bone.h"

Bone::Bone()
{
}

_matrix Bone::GetCombinedTransformationMatrix()
{
    return XMLoadFloat4x4(&m_CombinedTransformationMatrix);
}

const _float4x4* Bone::GetCombinedTransformationMatrixPtr()
{
    return &m_CombinedTransformationMatrix;
}

void Bone::SetTransformationMatrix(_fmatrix TransformationMatrix)
{
    XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
}

HRESULT Bone::Initialize(const NodeData* pNode, _int iParentIndex)
{
    m_iParentBoneIndex = iParentIndex;

    strcpy_s(m_szName, pNode->name.c_str());

    // NAS_Node의 transform은 이미 _float4x4 라고 가정
    m_TransformationMatrix = pNode->transform;

    // 행렬을 DirectXMath 스타일에 맞게 전치
    XMStoreFloat4x4(&m_TransformationMatrix,
        XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));

    XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

    return S_OK;
}

_bool Bone::CompareName(const _char* pName)
{
    return !strcmp(pName, m_szName);
}

void Bone::UpdateCombinedTransformationMatrix(const vector<Bone*>& Bones, _fmatrix PreTransformationMatrix)
{
    if (-1 == m_iParentBoneIndex)
    {
        XMStoreFloat4x4(&m_CombinedTransformationMatrix,
            XMLoadFloat4x4(&m_TransformationMatrix) * PreTransformationMatrix);
    }
    else
    {
        XMStoreFloat4x4(&m_CombinedTransformationMatrix,
            XMLoadFloat4x4(&m_TransformationMatrix) *
            XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
    }
}

Bone* Bone::Create(const NodeData* pNode, _int iParentIndex)
{
    Bone* pInstance = new Bone();

    if (FAILED(pInstance->Initialize(pNode, iParentIndex)))
    {
        MSG_BOX("Failed to Created : Bone");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Bone* Bone::Clone()
{
    return new Bone(*this);
}

void Bone::Free()
{
    __super::Free();
}