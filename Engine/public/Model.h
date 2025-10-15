#pragma once

#include "Component.h"
#include "NoAssimpModelStruct.h"

NS_BEGIN(Engine)

class ENGINE_DLL Model final : public Component
{
protected:
	Model();
	Model(const Model& Prototype);
	virtual ~Model() = default;

public:
	_uint GetNumMeshes() const;
	const _float4x4* GetSocketBoneMatrixPtr(const _char* pBoneName) const;

	virtual HRESULT InitializePrototype(MODELTYPE eType, const _char* pFilePath, _fmatrix PreTransformMatrix);

	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render(_uint iMeshIndex);

	HRESULT BindShaderResource(_uint iMeshIndex, class Shader* pShader, const _char* pConstantName, TextureType eType, _uint iIndex);
	HRESULT BindBoneMatrices(_uint iMeshIndex, class Shader* pShader, const _char* pConstantName);

	void PlayAnimation(_float fTimeDelta);
	void SetAnimation(_uint iIndex, _bool isLoop = false);
	_bool isAnimFinished() const;

	ModelData* GetModelData() const;
	void SetModelData(ModelData* pModelData);
	void SetTargetMesh(_int iMeshIndex);
	
	ModelData* LoadNoAssimpModel(const _char* pFilePath);

	static Model* Create(MODELTYPE eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	static Model* Create(MODELTYPE eType, ModelData* pModelData, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
private:
	HRESULT ReadyMeshes();
	HRESULT ReadyMaterials();
	HRESULT ReadyBones(const NodeData& nodeData, _int iParentIndex);
	HRESULT ReadyAnimations();
	void ClearModelData();

	HRESULT InitializePrototype(MODELTYPE eType, ModelData* pModelData, _fmatrix PreTransformMatrix);
private:
	ModelData*				m_pModelData = { nullptr };
	MODELTYPE				m_eType = { };
	_float4x4				m_PreTransformMatrix = {};

	_uint					m_iNumMeshes = {};
	vector<class Mesh*>	m_Meshes;

	_uint						m_iNumMaterials = {};
	vector<class Material*>	m_Materials;

	vector<class Bone*>		m_Bones;

	_bool						m_isAnimLoop = { false };
	_bool						m_isAnimFinished = { false };
	_uint						m_iCurrentAnimIndex = {};
	_uint						m_iNumAnimations = {};
	vector<class Animation*>	m_Animations;

	_int m_iTargetMeshIndex = -1;
};

NS_END