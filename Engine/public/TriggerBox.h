#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL TriggerBox final : public Base
{
    TriggerBox();
    virtual ~TriggerBox() = default;

public:
    HRESULT Initialize(void* pArg);
    void SetTriggerFunction(const function<void()>& func);
    
    void UpdateTrigger();
#ifdef _DEBUG
    HRESULT RenderTriggerBox();
#endif
    const TRIGGERBOX_DESC& GetTriggerBoxDesc();
    void UpdateFromDesc(const TRIGGERBOX_DESC& desc);

    void SetTriggerTag(const _wstring& triggerTag);

    static TriggerBox* Create(void* pArg);
    virtual void       Free() override;

private:
    TRIGGERBOX_DESC m_desc{};
    class EngineUtility* m_pEngineUtility = { nullptr };
    class CollisionBoxAABB* m_pCollisionBoxAABB = { nullptr };
    function<void()> m_TriggerFunction;

    _bool isTrigger = false;

#ifdef _DEBUG
    ID3D11InputLayout* m_pInputLayout = { nullptr };
    BasicEffect* m_pEffect = { nullptr };
    PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
#endif
};

NS_END