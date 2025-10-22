#include "Channel.h"
#include "Bone.h"

Channel::Channel()
{
}

HRESULT Channel::Initialize(const string& name, const vector<ChannelData>& channelData, const vector<class Bone*>& bones)
{
    strcpy_s(m_szName, name.c_str());

    auto iter = find_if(bones.begin(), bones.end(), [&](Bone* pBone)->_bool {
        if (pBone->CompareName(m_szName))
            return true;
        ++m_iBoneIndex;
        return false;
        });

    if (iter == bones.end())
     {
        OutputDebugStringA(("Channel::Initialize - Bone not found for channel: " + string(m_szName) + "\n").c_str());
        return E_FAIL;
    }

    // 해당 본 이름과 일치하는 채널을 찾기
    auto chIter = find_if(channelData.begin(), channelData.end(),
        [&](const ChannelData& data) { return data.nodeName == m_szName; });

    if (chIter == channelData.end())
    {
        OutputDebugStringA(("Channel::Initialize - ChannelData not found for: " + string(m_szName) + "\n").c_str());
        return E_FAIL;
    }

    m_ChannelData = *chIter;
    return S_OK;
}

void Channel::UpdateTransformationMatrix(_float fCurrentTrackPosition, const vector<Bone*>& Bones, _uint* pCurrentKeyFrameIndex)
{
    if (m_ChannelData.positionKeys.empty() ||
        m_ChannelData.rotationKeys.empty() ||
        m_ChannelData.scalingKeys.empty())
        return;

    if (0.0f == fCurrentTrackPosition)
        (*pCurrentKeyFrameIndex) = 0;

    // ======== 스케일 보간 ========
    XMVECTOR vScale{};
    {
        const auto& keys = m_ChannelData.scalingKeys;
        if (fCurrentTrackPosition <= keys.front().time)
            vScale = XMLoadFloat3(&keys.front().value);
        else if (fCurrentTrackPosition >= keys.back().time)
            vScale = XMLoadFloat3(&keys.back().value);
        else
        {
            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                if (fCurrentTrackPosition < keys[i + 1].time)
                {
                    float t = (fCurrentTrackPosition - keys[i].time) / (keys[i + 1].time - keys[i].time);
                    XMVECTOR v1 = XMLoadFloat3(&keys[i].value);
                    XMVECTOR v2 = XMLoadFloat3(&keys[i + 1].value);
                    vScale = XMVectorLerp(v1, v2, t);
                    break;
                }
            }
        }
    }

    // ======== 회전 보간 (Quaternion Slerp) ========
    XMVECTOR vRotation{};
    {
        const auto& keys = m_ChannelData.rotationKeys;
        if (fCurrentTrackPosition <= keys.front().time)
            vRotation = XMLoadFloat4(&keys.front().value);
        else if (fCurrentTrackPosition >= keys.back().time)
            vRotation = XMLoadFloat4(&keys.back().value);
        else
        {
            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                if (fCurrentTrackPosition < keys[i + 1].time)
                {
                    float t = (fCurrentTrackPosition - keys[i].time) / (keys[i + 1].time - keys[i].time);
                    XMVECTOR q1 = XMLoadFloat4(&keys[i].value);
                    XMVECTOR q2 = XMLoadFloat4(&keys[i + 1].value);
                    vRotation = XMQuaternionSlerp(q1, q2, t);
                    break;
                }
            }
        }
    }

    // ======== 위치 보간 ========
    XMVECTOR vTranslation{};
    {
        const auto& keys = m_ChannelData.positionKeys;
        if (fCurrentTrackPosition <= keys.front().time)
            vTranslation = XMLoadFloat3(&keys.front().value);
        else if (fCurrentTrackPosition >= keys.back().time)
            vTranslation = XMLoadFloat3(&keys.back().value);
        else
        {
            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                if (fCurrentTrackPosition < keys[i + 1].time)
                {
                    float t = (fCurrentTrackPosition - keys[i].time) / (keys[i + 1].time - keys[i].time);
                    XMVECTOR v1 = XMLoadFloat3(&keys[i].value);
                    XMVECTOR v2 = XMLoadFloat3(&keys[i + 1].value);
                    vTranslation = XMVectorLerp(v1, v2, t);
                    break;
                }
            }
        }
    }

    // ======== 최종 변환 행렬 구성 ========
    _matrix TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorZero(), vRotation, vTranslation);

    Bones[m_iBoneIndex]->SetTransformationMatrix(TransformationMatrix);
}

const char* Channel::GetName()
{
    return m_szName;
}

Channel* Channel::Create(const string& name, const vector<ChannelData>& channelData, const vector<class Bone*>& bones)
{
    Channel* pInstance = new Channel();

    if (FAILED(pInstance->Initialize(name, channelData, bones)))
    {
        MSG_BOX("Failed to Created : Channel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Channel::Free()
{
    __super::Free();
}
