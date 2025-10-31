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
    , m_Channels{ Prototype.m_Channels }
{
    for (_uint i = 0; i < m_iNumChannels; i++) {
        SafeAddRef(m_Channels[i]);
    }
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
        if (pChannel == nullptr)
            continue;
        m_Channels.push_back(pChannel);
    }

    return S_OK;
}

void Animation::UpdateTransformationMatrix(
    _float fTimeDelta,
    const vector<Bone*> bones,
    _bool isLoop,
    _bool* pFinished,
    ModelData* pModelData /* 🔹추가: 노드용 애니메이션을 적용하기 위함 */
)
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

    _uint iIndex = 0;
    for (auto& pChannel : m_Channels)
    {
        // 기존: 본 찾고 적용
        if (bones.empty() == false)
        {
            pChannel->UpdateTransformationMatrix(
                m_fCurrentTrackPosition,
                bones,
                &m_CurrentChannelIndex[iIndex++]
            );
            continue;
        }

        // 🔹 추가: 본이 없을 경우 (mNumBones == 0) → NodeData 애니메이션 적용
        if (pModelData)
        {
            // 채널 이름과 동일한 노드 탐색
            std::function<void(NodeData&)> applyAnim = [&](NodeData& node)
                {
                    if (node.name == pChannel->GetName())
                    {
                        _float4x4 animTransform;
                        XMStoreFloat4x4(&animTransform, pChannel->CalcInterpolatedTransform(m_fCurrentTrackPosition));

                        node.transform = animTransform;
                        return;
                    }

                    for (auto& child : node.children)
                        applyAnim(child);
                };

            applyAnim(pModelData->rootNode);
        }

        iIndex++;
    }
}
void Animation::Reset()
{
    m_fCurrentTrackPosition = 0.f;
    for (auto& idx : m_CurrentChannelIndex)
        idx = 0;
}

_float Animation::GetCurTrackPos() const
{
    return m_fCurrentTrackPosition;
}

_float Animation::GetDuration() const
{
    return m_fDuration;
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
