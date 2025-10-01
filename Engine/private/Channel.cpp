#include "Channel.h"

#include "Bone.h"

Channel::Channel()
{
}

HRESULT Channel::Initialize(const string& name, const vector<KEYFRAME>& keyframes, const vector<class Bone*>& Bones)
{
    strcpy_s(m_szName, name.c_str());

    auto iter = find_if(Bones.begin(), Bones.end(), [&](Bone* pBone)->_bool {
        if (pBone->CompareName(m_szName))
            return true;
        ++m_iBoneIndex;
        return false;
        });

    if (iter == Bones.end())
        return E_FAIL;

    m_KeyFrames = keyframes;
    m_iNumKeyFrames = static_cast<_uint>(keyframes.size());

    return S_OK;
}

void Channel::UpdateTransformationMatrix(_float fCurrentTrackPosition, const vector<Bone*>& Bones, _uint* pCurrentKeyFrameIndex)
{
    if (0.0f == fCurrentTrackPosition)
        (*pCurrentKeyFrameIndex) = 0;

    _vector vScale, vRotation, vTranslation;

    KEYFRAME LastKeyFrame = m_KeyFrames.back();

    if (fCurrentTrackPosition >= LastKeyFrame.fTrackPosition)
    {
        vScale = XMLoadFloat3(&LastKeyFrame.vScale);
        vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
        vTranslation = XMVectorSetW(XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
    }
    else
    {
        while (fCurrentTrackPosition >= m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].fTrackPosition)
            ++(*pCurrentKeyFrameIndex);

        _vector vSrcScale = XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex)].vScale);
        _vector vSrcRotation = XMLoadFloat4(&m_KeyFrames[(*pCurrentKeyFrameIndex)].vRotation);
        _vector vSrcTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex)].vTranslation), 1.f);

        _vector vDstScale = XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].vScale);
        _vector vDstRotation = XMLoadFloat4(&m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].vRotation);
        _vector vDstTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].vTranslation), 1.f);

        _float fRatio = (fCurrentTrackPosition - m_KeyFrames[(*pCurrentKeyFrameIndex)].fTrackPosition) /
            (m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].fTrackPosition - m_KeyFrames[(*pCurrentKeyFrameIndex)].fTrackPosition);

        vScale = XMVectorLerp(vSrcScale, vDstScale, fRatio);
        vRotation = XMQuaternionSlerp(vSrcRotation, vDstRotation, fRatio);
        vTranslation = XMVectorLerp(vSrcTranslation, vDstTranslation, fRatio);
    }

    _matrix TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vTranslation);

    Bones[m_iBoneIndex]->SetTransformationMatrix(TransformationMatrix);
}

Channel* Channel::Create(const string& name, const vector<KEYFRAME>& keyframes, const vector<class Bone*>& Bones)
{
    Channel* pInstance = new Channel();

    if (FAILED(pInstance->Initialize(name, keyframes, Bones)))
    {
        MSG_BOX("Failed to Created : Channel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Channel::Free()
{
    __super::Free();
    m_KeyFrames.clear();
}
