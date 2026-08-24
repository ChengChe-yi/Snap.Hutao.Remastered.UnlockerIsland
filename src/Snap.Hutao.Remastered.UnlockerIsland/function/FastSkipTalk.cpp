#include "FastSkipTalk.h"
#include "../framework.h"
#include "../dllmain.h"
#include "../Logger.h"
#include "HooksShared.h"

const char fastSkipTalkPatchBytes[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

void FastSkipTalk::Initialize()
{
    if (g_pEnv->Offsets.FastSkipTalk)
    {
        LPVOID addr = GetFunctionAddress(g_pEnv->Offsets.FastSkipTalk);
        if (addr)
        {
            this->patch = new Patch(addr, fastSkipTalkPatchBytes, 6);
            this->m_lastState = g_pEnv->FastSkipTalk != FALSE;
            this->patch->SetIsPatched(this->m_lastState);
            if (this->m_lastState)
                Log("FastSkipTalk: dialogue cooldown NOP applied.");
        }
    }
}

void FastSkipTalk::OnUpdate()
{
    if (this->patch)
    {
        bool enabled = g_pEnv->FastSkipTalk != FALSE;
        if (enabled != this->m_lastState)
        {
            this->m_lastState = enabled;
            this->patch->SetIsPatched(enabled);
            if (enabled)
                Log("FastSkipTalk: dialogue cooldown NOP applied.");
            else
                Log("FastSkipTalk: dialogue cooldown NOP reverted.");
        }
    }
}

void* FastSkipTalk::GetHookFunction()
{
    return nullptr;
}

bool FastSkipTalk::IsEnabled()
{
    return g_pEnv->FastSkipTalk != FALSE;
}
