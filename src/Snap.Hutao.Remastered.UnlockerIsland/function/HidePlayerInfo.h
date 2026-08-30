#pragma once

#include "IFunction.h"
#include "FunctionType.h"

class HidePlayerInfo : public IFunction
{
public:
    void Initialize() override;
    void OnUpdate() override;
    void* GetHookFunction() override { return nullptr; }
    bool IsEnabled() override;
    void SetEnabled(bool enabled) override { (void)enabled; }
    FunctionType GetFunctionType() override { return FunctionType::HIDE_PLAYER_INFO; }

    // Event-driven hiding: called from the SetWaterMaskUID hook right after
    // the original runs. Hides the watermark UID object immediately.
    static void HideUidWatermark();
};
