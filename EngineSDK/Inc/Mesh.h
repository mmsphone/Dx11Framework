#pragma once

#include "VertexIndexBuffer.h"
#include "NoAssimpModelStruct.h"

NS_BEGIN(Engine)

class ENGINE_DLL Mesh final : public VertexIndexBuffer
{
private:
	Mesh();
	Mesh(const Mesh& Prototype);
	virtual ~Mesh() = default;

public:
	_uint GetMaterialIndex() const;

	void SetVisible(_bool bVisible);
	_bool IsVisible();

	virtual HRESULT InitializePrototype(MODELTYPE eType, const MeshData& mesh, const vector<class Bone*>& Bones, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

	HRESULT BindBoneMatrices(class Shader* pShader, const _char* pConstantName, const vector<Bone*>& Bones);

	HRESULT ReadyVertexBufferForNonAnim(const MeshData& mesh, _fmatrix PreTransformMatrix);
	HRESULT ReadyVertexBufferForAnim(const MeshData& mesh, const vector<class Bone*>& Bones);

	HRESULT UpdateNonAnimVertexBuffer(const MeshData& mesh, _fmatrix PreTransformMatrix);

	static Mesh* Create(MODELTYPE eType, const MeshData& mesh, const vector<class Bone*>& Bones, _fmatrix PreTransformMatrix);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;

private:
	_char			m_szName[MAX_PATH] = {};
	/* 이 메시가 이용해야할  Material을 의미한다. */
	_uint			m_iMaterialIndex = {};
	_uint			m_iNumBones = { };

	/* 이 메시에 영향을 주는 뼈들의 인덱스(전체 모델 기준(CModel::m_Bones)) 를 모아둔다. */
	vector<_uint>	m_BoneIndices;
	_float4x4		m_BoneMatrices[g_iMaxNumBones] = {};
	vector<_float4x4>	m_OffsetMatrices;

	_bool m_bVisible = true;
};

NS_END