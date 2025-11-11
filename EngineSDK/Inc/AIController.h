#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class Object;
class Transform;

using AIValue = std::variant<_bool, _float, _uint, std::string, _vector, _matrix>;

typedef struct tagAIInputDesc
{
    std::unordered_map<std::string, AIValue> dataMap;
    const AIValue* GetPtr(const std::string& key) const { auto it = dataMap.find(key); return it == dataMap.end() ? nullptr : &it->second; }
    void SetData(const std::string& key, const AIValue& v) { dataMap[key] = v; }
    void Clear() noexcept { dataMap.clear(); }
} AIINPUT_DESC;

typedef struct tagAIOutputDesc
{
    std::unordered_map<std::string, AIValue> dataMap;
    const AIValue* GetPtr(const std::string& key) const { auto it = dataMap.find(key); return it == dataMap.end() ? nullptr : &it->second; }
    void SetData(const std::string& key, const AIValue& v) { dataMap[key] = v; }
    void Clear() noexcept { dataMap.clear(); }
} AIOUTPUT_DESC;

typedef struct tagAIProcessDesc
{
    std::function<void(AIINPUT_DESC& in, _float fTimeDelta, _float time)> sense;
    std::function<void(const AIINPUT_DESC& in, AIOUTPUT_DESC& out, _float fTimeDelta, _float time)> decide;
    std::function<void(AIOUTPUT_DESC& out, _float fTimeDelta, _float time)> act;
    std::function<void(const AIOUTPUT_DESC& out)> applyOutput;
} AIPROCESS_DESC;

class ENGINE_DLL AIController final : public Component
{
    AIController();
    AIController(const AIController& Prototype);
    virtual ~AIController() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    void Update(_float fTimeDelta);

    void BindProcessDesc(const AIPROCESS_DESC& processDesc);
    void SetInput(const AIINPUT_DESC& inputDesc);
    const AIOUTPUT_DESC& GetLastOutput() const;
    _float GetTime() const;

    static AIController* Create();
    virtual Component* Clone(void* pArg) override;
    virtual void Free() override;

private:
    AIINPUT_DESC   m_lastInput{};
    AIOUTPUT_DESC  m_lastOutput{};
    AIPROCESS_DESC m_process{};

    _float m_time = 0.f;
};

NS_END