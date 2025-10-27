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
    HRESULT Initialize(const string& name, const vector<ChannelData>& channelData, const vector<class Bone*>& bones);
    void UpdateTransformationMatrix(_float fCurrentTrackPosition, const vector<class Bone*>& Bones, _uint* pCurrentKeyFrameIndex);

    const char* GetName();

    static Channel* Create(const string& name, const vector<ChannelData>& channelData, const vector<class Bone*>& bones);
    virtual void Free() override;

    _matrix CalcInterpolatedTransform(_float fTimeDelta);

private:
    char                        m_szName[MAX_PATH] = "";
    _uint                       m_iBoneIndex = 0;

    // 채널 데이터: 해당 본의 모든 키프레임 정보
    ChannelData                 m_ChannelData;
};

NS_END
