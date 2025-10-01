#pragma once

#include "Base.h"
#include "NoAssimpModelStruct.h"

NS_BEGIN(Engine)

class Bone final : public Base
{
private:
    Bone();
    virtual ~Bone() = default;

public:
    _matrix GetCombinedTransformationMatrix();
    const _float4x4* GetCombinedTransformationMatrixPtr();

    void SetTransformationMatrix(_fmatrix TransformationMatrix);

    HRESULT Initialize(const NodeData* pNode, _int iParentIndex);
    _bool CompareName(const _char* pName);
    void UpdateCombinedTransformationMatrix(const vector<Bone*>& Bones, _fmatrix PreTransformationMatrix);

    static Bone* Create(const NodeData* pNode, _int iParentIndex);
    Bone* Clone();
    virtual void Free() override;
private:
    _char      m_szName[MAX_PATH] = {};
    _float4x4  m_TransformationMatrix = {};
    _float4x4  m_CombinedTransformationMatrix = {};
    _int       m_iParentBoneIndex = { -1 };
};

NS_END
