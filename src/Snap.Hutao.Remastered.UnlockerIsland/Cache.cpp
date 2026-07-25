#include "Cache.h"
#include "function/HooksShared.h"
#include "utils/UnityUtils.h"
#include "Logger.h"
#include "Constants.h"

bool g_cachedIsResisted = false;

typedef Il2CppString* (*FindStringFn)(const char*);
typedef void* (*GetComponentFn)(void*, Il2CppString*);
typedef void (*SetTextFn)(void*, Il2CppString*);

Il2CppString* GetText(void* pText)
{
    return *(Il2CppString**)((uintptr_t)pText + 0xE0);
}

bool CacheResistState()
{
    if (findString && getComponent)
    {
        FindStringFn findStringFunc = (FindStringFn)findString;
        GetComponentFn getComponentFunc = (GetComponentFn)getComponent;
		SetTextFn setTextFunc = (SetTextFn)setText;

        void* uidObj = FindGameObject(UID_PATH);
        if (!uidObj)
        {
            Log("Failed to find UID GameObject for resist check");
            g_cachedIsResisted = false;
            return false;
        }

        Log("UID GameObject was found for resist check");

        Il2CppString* textStr = findStringFunc("Text");
        if (textStr)
        {
            void* textComp = getComponentFunc(uidObj, textStr);

            if (textComp)
            {
                Log("Text component was found for resist check");
                Il2CppString* textContent = GetText(textComp);
                Log(textContent);

                if (textContent)
                {
                    bool hasGuid = wcsstr(textContent->chars, L"GUID") != nullptr;
                    bool isResisted = hasGuid;
                    if (isResisted)
                    {
						for (auto& whiteListItem : BeyondWhiteList)
						{
							if (wcsstr(textContent->chars, whiteListItem.c_str()) != nullptr)
							{
								isResisted = false;
								Log(("GUID whitelist match found"));
								break;
							}
						}
                    }

                    // If in whitelisted Beyond realm, replace #FFFFFF99 with green
                    if (hasGuid && !isResisted && setText && findString)
                    {
                        wchar_t* p = wcsstr(textContent->chars, BEYOND_TEXT_COLOR_ORIGINAL);
                        if (p)
                        {
                            std::wstring newText(textContent->chars);
                            size_t colorLen = wcslen(BEYOND_TEXT_COLOR_ORIGINAL);
                            for (size_t pos = newText.find(BEYOND_TEXT_COLOR_ORIGINAL); pos != std::wstring::npos; pos = newText.find(BEYOND_TEXT_COLOR_ORIGINAL, pos))
                            {
                                newText.replace(pos, colorLen, BEYOND_TEXT_COLOR_GREEN);
                                pos += colorLen;
                            }

                            int len = WideCharToMultiByte(CP_UTF8, 0, newText.c_str(), -1, nullptr, 0, nullptr, nullptr);
                            std::string utf8(len - 1, '\0');
                            WideCharToMultiByte(CP_UTF8, 0, newText.c_str(), -1, &utf8[0], len, nullptr, nullptr);

                            Il2CppString* newStr = ((FindStringFn)findString)(utf8.c_str());
                            if (newStr)
                            {
                                setTextFunc(textComp, newStr);
                            }
                        }
                    }

                    g_cachedIsResisted = isResisted;

                    if (isResisted)
                    {
                        Log("Resist state detected: GUID found in text");
                    }
                    return isResisted;
                }
            } 
            else 
            {
                Log("Text component was not found for resist check");
            }
        }
    }

    g_cachedIsResisted = false;
    return false;
}

bool CheckResistInBeyd()
{
    return g_cachedIsResisted;
}
