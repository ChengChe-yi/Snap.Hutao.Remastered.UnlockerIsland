#pragma once

#include "IFunction.h"
#include "FunctionType.h"

class HidePlayerInfo : public IFunction
{
public:
    void Initialize() override;
    void OnUpdate() override;
    void* GetHookFunction() override;
    bool IsEnabled() override;
    void SetEnabled(bool enabled) override { (void)enabled; }
    FunctionType GetFunctionType() override { return FunctionType::HIDE_PLAYER_INFO; }

    // Event-driven hiding: called from the SetWaterMaskUID hook right after
    // the original runs. Hides the watermark UID object immediately.
    static void HideUidWatermark();

    // Event-driven hiding: called from the SetupPlayerProfilePage hook after
    // the original has opened the page. Hides the player-identity objects
    // (profile UID/name/birthday) instead of blocking the whole page.
    static void HideProfileInfo();

    // SetupPlayerProfilePage hook: origin always runs so the page opens
    // normally, then the identity objects are hidden (see HideProfileInfo).
    static void HookSetupPlayerProfilePage(void* pThis);
};
