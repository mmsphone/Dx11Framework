#include "Animation.h"

#include "Bone.h"

Animation::Animation()
{
}

Animation::Animation(const Animation& Prototype)
    : m_fDuration{ Prototype.m_fDuration }
    , m_fCurrentTrackPosition{ Prototype.m_fCurrentTrackPosition }
    , m_fTicksPerSecond{ Prototype.m_fTicksPerSecond }
    , m_iNumChannels{ Prototype.m_iNumChannels }
    , m_CurrentKeyFrameIndex{ Prototype.m_CurrentKeyFrameIndex }
{
}

HRESULT Animation::Initialize(const AnimationData& animationData, const vector<Bone*> bones)
{
    m_iNumChannels = static_cast<_uint>(bones.size()); // 각 본마다 채널이 있다고 가정
    m_fDuration = animationData.duration;
    m_fTicksPerSecond = animationData.ticksPerSecond;

    m_CurrentKeyFrameIndex.resize(m_iNumChannels, 0);

    // 현재 구조체 기반에서는 실제 키프레임 채널 로딩은 생략
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

    // 실제 키프레임 기반 본 변환 계산은 구조체 기반에서는 생략 가능
    // 필요시 boneMatrices와 AnimationData를 이용하여 업데이트 로직 구현
}

void Animation::Reset()
{
    m_fCurrentTrackPosition = 0.f;
    for (auto& idx : m_CurrentKeyFrameIndex)
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
    m_CurrentKeyFrameIndex.clear();
}
