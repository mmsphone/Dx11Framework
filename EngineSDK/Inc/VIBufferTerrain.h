#pragma once

#include "VertexIndexBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL VIBufferTerrain final : public VertexIndexBuffer
{
private:
	VIBufferTerrain();
	VIBufferTerrain(const VIBufferTerrain& Prototype);
	virtual ~VIBufferTerrain() = default;

public:
	virtual HRESULT InitializePrototype(const _tchar* pHeightMapFilePath);
	virtual HRESULT Initialize(void* pArg) override;

	float GetHeightAt(float x, float z) const;
	const vector<vector<float>>& GetHeightMap() const;

	_uint GetNumVerticesX() const;
	_uint GetNumVerticesZ() const;

	static VIBufferTerrain* Create(const _tchar* pHeightMapFilePath);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;

private:
	HRESULT LoadHeightMap(const std::wstring& heightMapPath);

private:
	vector<vector<float>> m_HeightMapData;
	_uint			m_iNumVerticesX = {};
	_uint			m_iNumVerticesZ = {};
};

NS_END