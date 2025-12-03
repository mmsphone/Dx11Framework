// SoundManager.cpp
#include "SoundManager.h"
#include <fmod.hpp>
#include <fmod_errors.h>

SoundManager::SoundManager() {}
SoundManager::~SoundManager() {}

SoundManager* SoundManager::Create()
{
    SoundManager* p = new SoundManager();
    if (!p->Initialize()) {
        SafeRelease(p);
        return nullptr;
    }
    return p;
}

bool SoundManager::Initialize()
{
    FMOD_RESULT res = FMOD::System_Create(&m_pSystem);
    if (res != FMOD_OK) return false;

    // 3D 설정 (채널 수는 적당히)
    res = m_pSystem->init(512, FMOD_INIT_3D_RIGHTHANDED, nullptr);
    if (res != FMOD_OK) return false;

    // 3D 설정값 (distance factor 등) 필요하면 여기서 세팅
    // m_pSystem->set3DSettings(1.0f, 1.0f, 1.0f);

    return true;
}

void SoundManager::Free()
{
    m_channels.clear();

    for (auto& kv : m_sounds) {
        if (kv.second)
            kv.second->release();
    }
    m_sounds.clear();

    if (m_pSystem) {
        m_pSystem->close();
        m_pSystem->release();
        m_pSystem = nullptr;
    }
}

void SoundManager::Update()
{
    if (m_pSystem)
        m_pSystem->update();

    if (!m_callbackEntries.empty())
    {
        for (size_t i = 0; i < m_callbackEntries.size(); )
        {
            CallbackEntry& e = m_callbackEntries[i];

            bool playing = false;
            bool remove = false;

            if (!e.channel)
            {
                remove = true;
            }
            else
            {
                FMOD_RESULT r = e.channel->isPlaying(&playing);
                if (r != FMOD_OK || !playing)
                {
                    // 더 이상 재생 중이 아님 → 콜백 호출
                    if (e.onEnd)
                        e.onEnd();
                    remove = true;
                }
            }

            if (remove)
            {
                // 한 번 콜하면 제거 (중복 호출 방지)
                m_callbackEntries[i] = m_callbackEntries.back();
                m_callbackEntries.pop_back();
            }
            else
            {
                ++i;
            }
        }
    }
}

bool SoundManager::LoadSound(const std::string& key,
    const std::wstring& pathW,
    bool is3D, bool loop)
{
    if (!m_pSystem) return false;

    if (m_sounds.count(key))
        return true; // 이미 로드됨

    std::string path(pathW.begin(), pathW.end()); // 간단 변환 (유니코드 제대로 하려면 별도 처리)

    FMOD_MODE mode = 0;
    mode |= is3D ? FMOD_3D : FMOD_2D;
    mode |= loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

    FMOD::Sound* pSound = nullptr;
    FMOD_RESULT res = m_pSystem->createSound(path.c_str(), mode, nullptr, &pSound);
    if (res != FMOD_OK)
        return false;

    m_sounds[key] = pSound;
    return true;
}

void SoundManager::UnloadSound(const std::string& key)
{
    auto it = m_sounds.find(key);
    if (it == m_sounds.end())
        return;
    if (it->second)
        it->second->release();
    m_sounds.erase(it);
}

void SoundManager::Play2D(const std::string& key, float volume)
{
    if (!m_pSystem) return;
    auto it = m_sounds.find(key);
    if (it == m_sounds.end()) return;

    FMOD::Channel* ch = nullptr;
    m_pSystem->playSound(it->second, nullptr, false, &ch);
    if (ch)
    {
        ch->setVolume(volume);
        m_channels[key] = ch;   // ★ 마지막 채널 저장
    }
}

void SoundManager::Play3D(const std::string& key,
    float x, float y, float z,
    float volume)
{
    if (!m_pSystem) return;
    auto it = m_sounds.find(key);
    if (it == m_sounds.end()) return;

    FMOD::Channel* ch = nullptr;
    m_pSystem->playSound(it->second, nullptr, true, &ch); // 일단 pause 상태로 받음
    if (!ch) return;

    FMOD_VECTOR pos{ x, y, z };
    FMOD_VECTOR vel{ 0.f, 0.f, 0.f };
    ch->set3DAttributes(&pos, &vel);
    ch->setVolume(volume);
    ch->setPaused(false);

    m_channels[key] = ch;
}

void SoundManager::StopSound(const std::string& key)
{
    auto it = m_channels.find(key);
    if (it == m_channels.end())
        return;

    FMOD::Channel* ch = it->second;
    if (ch)
    {
        bool playing = false;
        if (ch->isPlaying(&playing) == FMOD_OK && playing)
            ch->stop();
    }

    m_channels.erase(it);
}

void SoundManager::Play2DWithCallback(const std::string& key, float volume, const std::function<void()>& onEnd)
{
    if (!m_pSystem) return;
    auto it = m_sounds.find(key);
    if (it == m_sounds.end()) return;

    FMOD::Channel* ch = nullptr;
    m_pSystem->playSound(it->second, nullptr, false, &ch);
    if (!ch) return;

    ch->setVolume(volume);
    m_channels[key] = ch;

    if (onEnd)
    {
        CallbackEntry e;
        e.channel = ch;
        e.onEnd = onEnd;
        m_callbackEntries.push_back(e);
    }
}

void SoundManager::Play3DWithCallback(const std::string& key, float x, float y, float z, float volume, const std::function<void()>& onEnd)
{
    if (!m_pSystem) return;
    auto it = m_sounds.find(key);
    if (it == m_sounds.end()) return;

    FMOD::Channel* ch = nullptr;
    m_pSystem->playSound(it->second, nullptr, true, &ch); // pause 상태
    if (!ch) return;

    FMOD_VECTOR pos{ x, y, z };
    FMOD_VECTOR vel{ 0.f, 0.f, 0.f };
    ch->set3DAttributes(&pos, &vel);
    ch->setVolume(volume);
    ch->setPaused(false);

    m_channels[key] = ch;

    if (onEnd)
    {
        CallbackEntry e;
        e.channel = ch;
        e.onEnd = onEnd;
        m_callbackEntries.push_back(e);
    }
}

void SoundManager::SetListener(const _float3& pos,
    const _float3& forward,
    const _float3& up,
    const _float3& vel)
{
    if (!m_pSystem) return;

    FMOD_VECTOR fmodPos{ pos.x, pos.y, pos.z };
    FMOD_VECTOR fmodFwd{ forward.x, forward.y, forward.z };
    FMOD_VECTOR fmodUp{ up.x, up.y, up.z };
    FMOD_VECTOR fmodVel{ vel.x, vel.y, vel.z };

    m_pSystem->set3DListenerAttributes(
        0, &fmodPos, &fmodVel, &fmodFwd, &fmodUp);
}
