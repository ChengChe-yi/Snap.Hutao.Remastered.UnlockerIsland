#pragma once

#include "IFunction.h"
#include "FunctionType.h"
#include "../utils/Patch.h"

class FastSkipTalk : public IFunction
{
public:
    void Initialize() override;
    void OnUpdate() override;
    void* GetHookFunction() override;
    bool IsEnabled() override;
    void SetEnabled(bool enabled) override { (void)enabled; }
    FunctionType GetFunctionType() override { return FunctionType::FAST_SKIP_TALK; }

private:
    Patch* patch = nullptr;
    bool m_lastState = false;
};
