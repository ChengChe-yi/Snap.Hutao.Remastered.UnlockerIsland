#include "BeyondResist.h"
#include "Logger.h"
#include "Constants.h"

bool g_cachedIsResisted = false;

static bool IsBeyondWhitelisted(Il2CppString* textContent)
{
    for (auto& whiteListItem : BeyondWhiteList)
    {
        if (wcsstr(textContent->chars, whiteListItem.c_str()) != nullptr)
        {
            Log("GUID whitelist match found");
            return true;
        }
    }
    return false;
}

// Replaces every #FFFFFF99 with #00FF00FF in place. Both codes have the same
// length, so the replacement never overflows the string buffer.
static void ReplaceTextColorInPlace(Il2CppString* textContent)
{
    const size_t colorLen = wcslen(BEYOND_TEXT_COLOR_ORIGINAL);
    wchar_t* base = textContent->chars;
    for (;;)
    {
        wchar_t* p = wcsstr(base, BEYOND_TEXT_COLOR_ORIGINAL);
        if (!p)
        {
            break;
        }
        memcpy(p, BEYOND_TEXT_COLOR_GREEN, colorLen * sizeof(wchar_t));
        base = p + colorLen;
    }
}

// String-driven resist check: detect 千星奇域 directly from the UID text,
// update g_cachedIsResisted, and apply the whitelist green color in place.
// A missing or GUID-free UID text means the player left 千星奇域 → lifted.
bool CacheResistState(Il2CppString* textContent)
{
    const bool wasResisted = g_cachedIsResisted;

    bool hasGuid = false;
    bool isResisted = false;

    if (textContent && textContent->chars)
    {
        hasGuid = wcsstr(textContent->chars, L"GUID") != nullptr;
        isResisted = hasGuid && !IsBeyondWhitelisted(textContent);

        if (hasGuid && !isResisted)
        {
            ReplaceTextColorInPlace(textContent);
        }
    }

    g_cachedIsResisted = isResisted;

    if (isResisted)
    {
        Log("Resist state detected");
    }
    else if (wasResisted)
    {
        Log("Resist lifted");
    }
    return isResisted;
}

bool CheckResistInBeyd()
{
    return g_cachedIsResisted;
}
