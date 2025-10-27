#pragma once

#include "Base.h"
#include "NoAssimpModelStruct.h"

NS_BEGIN(Engine)

class Animation final : public Base
{
private:
	Animation();
	Animation(const Animation& Prototype);
	virtual ~Animation() = default;

public:
	HRESULT Initialize(const AnimationData& animationData, const vector<class Bone*> bones);
	void UpdateTransformationMatrix(_float fTimeDelta, const vector<class Bone*> boneMatrices, _bool isLoop, _bool* pFinished, ModelData* pModelData);
	void Reset();
	_float GetCurTrackPos() const;
	_float GetDuration() const;

	static Animation* Create(const AnimationData& animationData, const vector<Bone*> bones);
	Animation* Clone();
	virtual void Free() override;

private:
	_float					m_fDuration = 0.f;
	_float					m_fCurrentTrackPosition = 0.f;
	_float					m_fTicksPerSecond = 0.f;

	_uint					m_iNumChannels = 0;
	vector<class Channel*>	m_Channels;
	vector<_uint>			m_CurrentChannelIndex;
};

NS_END
