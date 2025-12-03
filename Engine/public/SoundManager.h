// SoundManager.h
#pragma once

#include "Base.h"

namespace FMOD {
    class System;
    class Sound;
    class Channel;
}

NS_BEGIN(Engine)

class ENGINE_DLL SoundManager final : public Base
{
private:
    SoundManager();
    virtual ~SoundManager();

public:
    static SoundManager* Create();
    virtual void Free() override;

    bool Initialize();
    void Update(); // 매 프레임 호출 (system->update)

    // 리소스
    bool LoadSound(const std::string& key, const std::wstring& path, bool is3D = true, bool loop = false);
    void UnloadSound(const std::string& key);

    // 재생
    void Play2D(const std::string& key, float volume = 1.0f);
    void Play3D(const std::string& key, float x, float y, float z, float volume = 1.0f);
    void StopSound(const std::string& key);

    void Play2DWithCallback(const std::string& key, float volume, const std::function<void()>& onEnd);
    void Play3DWithCallback(const std::string& key, float x, float y, float z, float volume, const std::function<void()>& onEnd);
    
    // 리스너 설정 (카메라/플레이어)
    void SetListener(const _float3& pos, const _float3& forward, const _float3& up, const _float3& vel = _float3(0, 0, 0));

private:
    FMOD::System* m_pSystem = nullptr;
    std::unordered_map<std::string, FMOD::Sound*>   m_sounds;
    std::unordered_map<std::string, FMOD::Channel*> m_channels;

    struct CallbackEntry
    {
        FMOD::Channel* channel = nullptr;
        std::function<void()>       onEnd;
    };
    std::vector<CallbackEntry> m_callbackEntries;
};

NS_END
