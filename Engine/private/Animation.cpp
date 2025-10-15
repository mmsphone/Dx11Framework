#include "Animation.h"

#include "Channel.h"

Animation::Animation()
{
}

Animation::Animation(const Animation& Prototype)
    : m_fDuration{ Prototype.m_fDuration }
    , m_fCurrentTrackPosition{ Prototype.m_fCurrentTrackPosition }
    , m_fTicksPerSecond{ Prototype.m_fTicksPerSecond }
    , m_iNumChannels{ Prototype.m_iNumChannels }
    , m_CurrentChannelIndex{ Prototype.m_CurrentChannelIndex }
{
}

HRESULT Animation::Initialize(const AnimationData& animationData, const vector<Bone*> bones)
{
    m_iNumChannels = static_cast<_uint>(animationData.channels.size());
    m_fDuration = animationData.duration;
    m_fTicksPerSecond = animationData.ticksPerSecond;
    m_CurrentChannelIndex.resize(m_iNumChannels, 0);

    for (auto& chData : animationData.channels)
    {
        Channel* pChannel = Channel::Create(chData.nodeName, animationData.channels, bones);
        if (pChannel)
            m_Channels.push_back(pChannel);
    }

    return S_OK;
}

void Animation::UpdateTransformationMatrix(_float fTimeDelta, const vector<Bone*> bones, _bool isLoop, _bool* pFinished)
{
    m_fCurrentTrackPosition += m_fTicksPerSecond * fTimeDelta;

    if (m_fCurrentTrackPosition >= m_fDuration)
    {
        if (!isLoop)
        {
            *pFinished = true;
            return;
        }
        m_fCurrentTrackPosition = 0.f;
    }

    _uint       iIndex = {};
    for (auto& pChannel : m_Channels)
    {
        pChannel->UpdateTransformationMatrix(m_fCurrentTrackPosition, bones, &m_CurrentChannelIndex[iIndex++]);
    }
}

void Animation::Reset()
{
    m_fCurrentTrackPosition = 0.f;
    for (auto& idx : m_CurrentChannelIndex)
        idx = 0;
}

Animation* Animation::Create(const AnimationData& animationData, const vector<Bone*> bones)
{
    Animation* pInstance = new Animation();
    if (FAILED(pInstance->Initialize(animationData, bones)))
    {
        MSG_BOX("Failed to Created : Animation");
        SafeRelease(pInstance);
    }
    return pInstance;
}

Animation* Animation::Clone()
{
    return new Animation(*this);
}

void Animation::Free()
{
    __super::Free();

    for (auto& pChannel : m_Channels)
        SafeRelease(pChannel);
    m_Channels.clear();
}
