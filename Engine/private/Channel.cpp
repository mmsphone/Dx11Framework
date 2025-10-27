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

_matrix Channel::CalcInterpolatedTransform(_float fTimeDelta)
{
    // --- 위치(Position) ---
    _float3 vPos{ 0.f, 0.f, 0.f };

    if (!m_ChannelData.positionKeys.empty())
    {
        if (m_ChannelData.positionKeys.size() == 1)
            vPos = m_ChannelData.positionKeys[0].value;
        else
        {
            _uint iNext = 0;
            for (_uint i = 0; i < m_ChannelData.positionKeys.size() - 1; ++i)
            {
                if (fTimeDelta < m_ChannelData.positionKeys[i + 1].time)
                {
                    iNext = i + 1;
                    break;
                }
            }

            _uint iPrev = (iNext == 0) ? 0 : iNext - 1;
            _float t = (fTimeDelta - m_ChannelData.positionKeys[iPrev].time) /
                (m_ChannelData.positionKeys[iNext].time - m_ChannelData.positionKeys[iPrev].time);
            XMStoreFloat3(&vPos, XMVectorLerp(
                XMLoadFloat3(&m_ChannelData.positionKeys[iPrev].value),
                XMLoadFloat3(&m_ChannelData.positionKeys[iNext].value),
                t
            ));
        }
    }

    // --- 회전(Rotation) ---
    _float4 vRot{ 0.f, 0.f, 0.f, 1.f };
    if (!m_ChannelData.rotationKeys.empty())
    {
        if (m_ChannelData.rotationKeys.size() == 1)
            vRot = m_ChannelData.rotationKeys[0].value;
        else
        {
            _uint iNext = 0;
            for (_uint i = 0; i < m_ChannelData.rotationKeys.size() - 1; ++i)
            {
                if (fTimeDelta < m_ChannelData.rotationKeys[i + 1].time)
                {
                    iNext = i + 1;
                    break;
                }
            }

            _uint iPrev = (iNext == 0) ? 0 : iNext - 1;
            _float t = (fTimeDelta - m_ChannelData.rotationKeys[iPrev].time) /
                (m_ChannelData.rotationKeys[iNext].time - m_ChannelData.rotationKeys[iPrev].time);

            XMVECTOR q1 = XMLoadFloat4(&m_ChannelData.rotationKeys[iPrev].value);
            XMVECTOR q2 = XMLoadFloat4(&m_ChannelData.rotationKeys[iNext].value);
            XMVECTOR q = XMQuaternionSlerp(q1, q2, t);
            XMStoreFloat4(&vRot, q);
        }
    }

    // --- 스케일(Scale) ---
    _float3 vScale{ 1.f, 1.f, 1.f };
    if (!m_ChannelData.scalingKeys.empty())
    {
        if (m_ChannelData.scalingKeys.size() == 1)
            vScale = m_ChannelData.scalingKeys[0].value;
        else
        {
            _uint iNext = 0;
            for (_uint i = 0; i < m_ChannelData.scalingKeys.size() - 1; ++i)
            {
                if (fTimeDelta < m_ChannelData.scalingKeys[i + 1].time)
                {
                    iNext = i + 1;
                    break;
                }
            }

            _uint iPrev = (iNext == 0) ? 0 : iNext - 1;
            _float t = (fTimeDelta - m_ChannelData.scalingKeys[iPrev].time) /
                (m_ChannelData.scalingKeys[iNext].time - m_ChannelData.scalingKeys[iPrev].time);

            XMStoreFloat3(&vScale, XMVectorLerp(
                XMLoadFloat3(&m_ChannelData.scalingKeys[iPrev].value),
                XMLoadFloat3(&m_ChannelData.scalingKeys[iNext].value),
                t
            ));
        }
    }

    // --- 최종 행렬 구성 ---
    _matrix matScale = XMMatrixScaling(vScale.x, vScale.y, vScale.z);
    _matrix matRot = XMMatrixRotationQuaternion(XMLoadFloat4(&vRot));
    _matrix matTrans = XMMatrixTranslation(vPos.x, vPos.y, vPos.z);

    return matScale * matRot * matTrans;
}
