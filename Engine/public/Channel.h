#pragma once

#include "Base.h"
#include "NoAssimpModelStruct.h"

NS_BEGIN(Engine)

class Channel final : public Base
{
private:
	Channel();
	virtual ~Channel() = default;

public:
	HRESULT Initialize(const string& name, const vector<KEYFRAME>& keyframes, const vector<class Bone*>& Bones);
	void UpdateTransformationMatrix(_float fCurrentTrackPosition, const vector<Bone*>& Bones, _uint* pCurrentKeyFrameIndex);

	static Channel* Create(const string& name, const vector<KEYFRAME>& keyframes, const vector<class Bone*>& Bones);
	virtual void Free() override;
private:
	_char				m_szName[MAX_PATH] = {};
	_uint				m_iNumKeyFrames = 0;
	vector<KEYFRAME>	m_KeyFrames;

	_uint				m_iBoneIndex = 0;
};

NS_END
