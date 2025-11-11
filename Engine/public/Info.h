#pragma once

#include "Component.h"

NS_BEGIN(Engine)

using InfoValue = std::variant<_bool, _int, _float, _vector, _float3, _float4, std::string>;

typedef struct tagInfoDesc {
    std::unordered_map<std::string, InfoValue> dataMap;
    const InfoValue* GetPtr(const std::string& key) const { auto it = dataMap.find(key); return it == dataMap.end() ? nullptr : &it->second; }
    void SetData(const std::string& key, const InfoValue& v) { dataMap[key] = v; }
    void Clear() noexcept { dataMap.clear(); }
} INFO_DESC;

class ENGINE_DLL Info final : public Component
{
private:
    Info();
    Info(const Info& Prototype);
    virtual ~Info() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    void    Update(_float fTimeDelta);

    void BindInfoDesc(const INFO_DESC& infoDesc);
    INFO_DESC& GetInfo();
    void AddHp(_float hpAmount);
    _bool IsDead() const;

    static  Info* Create();
    virtual Component* Clone(void* pArg) override;
    virtual void           Free() override;

private:
    INFO_DESC m_infoDesc{};
};

NS_END